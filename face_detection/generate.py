#!/usr/bin/env python
#coding=utf-8
import cv2
import os  
import sys

face_cascade=cv2.CascadeClassifier('/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_default.xml')

def makedir(path):
    path=path.strip().rstrip('/')
    if os.path.exists(path) is False:
        os.makedirs( path );

def generate(root_argv,dirname):
    subject_dir_path = os.path.join(root_argv, dirname)
    print 'seek:'+subject_dir_path
    count=0
    for filename in os.listdir(subject_dir_path): 
        if filename == ".directory":
            continue
        imgPath = os.path.join(subject_dir_path, filename)
        try:
            print 'read:'+imgPath
            img = cv2.imread(imgPath)
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            outdir=os.path.join(root_argv,'generate',dirname)
            makedir(outdir)
            faces=face_cascade.detectMultiScale(gray,1.3,5)
            for x,y,w,h in faces:
                f=cv2.resize(gray[y:y+h,x:x+w],(200,200))
                outPath=os.path.join(root_argv,'generate',dirname,'%s.pgm' % str(count))
                print 'write:'+ outPath
                cv2.imwrite(outPath, f)
                count+=1

        except:
            pass


if __name__ == '__main__':
    if len(sys.argv)==1:
        print ("USAGE: generate.py <data>")
        exit(0)

    root_argv=sys.argv[1]
    for dirname in os.listdir(root_argv):
        file_path = os.path.join(root_argv, dirname)  
        if os.path.isdir(file_path): 
            if dirname == 'generate':
                continue
            generate(root_argv,dirname)
