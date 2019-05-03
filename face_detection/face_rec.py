#!/usr/bin/env python
#coding=utf-8
### Imports ###################################################################

import multiprocessing as mp
import cv2
import os
import sys
import time
import numpy as np
import threading
import Queue
import datetime

### Setup #####################################################################
### Global params###
resX = 320
resY = 240

# The face cascade file to be used
face_cascade = cv2.CascadeClassifier('/usr/local/share/OpenCV/lbpcascades/lbpcascade_frontalface.xml')

#model = cv2.createEigenFaceRecognizer()
#model = cv2.face.createEigenFaceRecognizer()
model = cv2.face.FisherFaceRecognizer_create()
#model = cv2.createLBPHFaceRecognizer()

t_start = time.time()
fps = 0

lock = threading.RLock()
queue = Queue.Queue(30)
camera = cv2.VideoCapture(0)
system_quit = False
names = ['devin', 'deng']

### Helper Functions ##########################################################
def normalize(X, low, high, dtype=None):
    """Normalizes a given array in X to a value between low and high."""
    X = np.asarray(X)
    minX, maxX = np.min(X), np.max(X)
    # normalize to [0...1].
    X = X - float(minX)
    X = X / float((maxX - minX))
    # scale to [low...high].
    X = X * (high-low)
    X = X + low
    if dtype is None:
        return np.asarray(X)
    return np.asarray(X, dtype=dtype)


def load_images(path, sz=None):
    c = 0
    X,y = [], []
    for dirname, dirnames, filenames in os.walk(path):
        for subdirname in dirnames:
            subject_path = os.path.join(dirname, subdirname)
            for filename in os.listdir(subject_path):
                try:
                    filepath = os.path.join(subject_path, filename)
                    if os.path.isdir(filepath):
                        continue
                    img = cv2.imread(os.path.join(subject_path, filename), cv2.IMREAD_GRAYSCALE)
                    if (img is None):
                        print ("image " + filepath + " is none")
                    else:
                        print (filepath)
                    # resize to given size (if given)
                    if (sz is not None):
                        img = cv2.resize(img, (200, 200))

                    X.append(np.asarray(img, dtype=np.uint8))
                    y.append(c)
                # except IOError, (errno, strerror):
                #     print ("I/O error({0}): {1}".format(errno, strerror))
                except:
                    print ("Unexpected error:", sys.exc_info()[0])
                    raise
            print ("c: " + str(c))
            c = c+1


    print (y)
    return [X,y]

def get_faces( img ):

    gray = cv2.cvtColor( img, cv2.COLOR_BGR2GRAY )
    faces = face_cascade.detectMultiScale(gray, 1.3, 5)

    return faces, img, gray

def draw_frame( faces, img, gray ):

    global xdeg
    global ydeg
    global fps
    global time_t
    global names

    # Draw a rectangle around every face
    for ( x, y, w, h ) in faces:

        cv2.rectangle( img, ( x, y ),( x + w, y + h ), ( 200, 255, 0 ), 2 )
#        img = cv2.rectangle(( x, y ),( x + w, y + h ), ( 200, 255, 0 ), 2)
        #-----rec-face
        roi = gray[x:x+w, y:y+h]
        try:
            roi = cv2.resize(roi, (200, 200), interpolation=cv2.INTER_LINEAR)
            params = model.predict(roi)
            sign=("%s %.2f" % (names[params[0]], params[1]))
            cv2.putText(img, sign, (x, y-2), cv2.FONT_HERSHEY_SIMPLEX, 0.5, ( 0, 0, 255 ), 2 )
            if (params[0] == 0):
                cv2.imwrite('face_rec.jpg', img)
        except:
            continue


    # Calculate and show the FPS
    fps = fps + 1
    sfps = fps / (time.time() - t_start)
    cv2.putText(img, "FPS : " + str( int( sfps ) ), ( 10, 15 ), cv2.FONT_HERSHEY_SIMPLEX, 0.5, ( 0, 0, 255 ), 2 )

    cv2.imshow( "recognize-face", img )


    
class ImageCapture(threading.Thread):
    """
    Camera capture
    """
    def __init__(self, t_name):  
        threading.Thread.__init__(self, name=t_name)  

    global queue
    global lock
    global camera
    global system_quit
    
        
    def run(self):
        cap_cnt = 0
        starttime = datetime.datetime.now()
        camera.set(cv2.CAP_PROP_FRAME_WIDTH,resX)  
        camera.set(cv2.CAP_PROP_FRAME_HEIGHT,resY) 
        
        while (system_quit == False):
            read, img = camera.read()
            cap_cnt += 1
            if lock.acquire():
                # put image to queue
                if queue.full() != True:
                    queue.put(img)
                lock.release()
        
        endtime = datetime.datetime.now()
        usedtime = abs(int(endtime.second) - int(starttime.second))
        fps = cap_cnt / usedtime
        
        print("capture fps: %d, %d"%(fps, usedtime))
        print(self.getName() + " exit, cap_cnt: %d"%(cap_cnt))
        camera.release()
 
class FaceDetection(threading.Thread):
    """
    Face detection
    """
    def __init__(self, t_name):  
        threading.Thread.__init__(self, name=t_name)  

    global queue
    global lock
    global camera
    global system_quit
    
    det_num = 0
    
    def run(self):
        
        pool = mp.Pool( processes=4 )
        
        [X,y] = load_images(sys.argv[1])
        y = np.asarray(y, dtype=np.int32)
        if len(sys.argv) == 3:
            out_dir = sys.argv[2]
        
        model.train(np.asarray(X), np.asarray(y))
        
        read, img = camera.read()
        pr1 = pool.apply_async( get_faces, [ img ] )   
        read, img = camera.read()
        pr2 = pool.apply_async( get_faces, [ img ] )  
        read, img = camera.read() 
        pr3 = pool.apply_async( get_faces, [ img ] )   
        read, img = camera.read()
        pr4 = pool.apply_async( get_faces, [ img ] ) 
        
        fcount = 1
        det_cnt = 0
        
        while (system_quit == False):
            if lock.acquire():
                if queue.empty() != True:
                    # get image from queue
                    img = queue.get()
                    lock.release()
                    if   fcount == 1:
                        pr1 = pool.apply_async( get_faces, [ img ] )
                        faces, img, gray=pr2.get()
                        draw_frame( faces, img, gray )

                    elif fcount == 2:
                        pr2 = pool.apply_async( get_faces, [ img ] )
                        faces, img, gray=pr3.get()
                        draw_frame( faces, img, gray )

                    elif fcount == 3:
                        pr3 = pool.apply_async( get_faces, [ img ] )
                        faces, img, gray=pr4.get()
                        draw_frame( faces, img, gray )

                    elif fcount == 4:
                        pr4 = pool.apply_async( get_faces, [ img ] )
                        faces, img, gray=pr1.get()
                        draw_frame( faces, img, gray )
                        fcount = 0

                    fcount += 1
                    det_cnt += 1
                    
                    # ord: return the ascii code
                    # waitkey: 
                    if cv2.waitKey(1000 // 12) & 0xff == ord("q"):
                        break
                else:
                    lock.release()
            
        print(self.getName() + " exit, det_cnt:%d "%(det_cnt))
        cv2.destroyAllWindows()
  
  
### Main ######################################################################

if __name__ == '__main__':
    thread_cature = ImageCapture('ImageCapture')
    thread_cature.start()
    time.sleep(0.5)
    thread_face_detection = FaceDetection('FaceDetection')
    thread_face_detection.start()             
    
    while (True):
        str = raw_input("Enter q to quit: ")
        if str == "q":
            system_quit = True
            break
      
    time.sleep(1)
    thread_face_detection.join()
    thread_cature.join()
    queue.queue.clear()
    

