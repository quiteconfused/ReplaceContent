#!/usr/bin/python
import os, sys, getopt
from subprocess import *
import readline
import string
import Queue
import threading

image_queue = Queue.Queue()
thread_queue = Queue.Queue()

def main(argv):
    usage =  '%s -f <inputfile> -c <configfile> [[--display] [-o <output file>]]' % sys.argv[0]
    inputfile = ''
    configfile = ''
    display = False
    outputfile = ''

    size_result = ''


    try:
        opts, args = getopt.getopt(argv,"hf:c:d:o:",["inputfile=","configfile=", "display", "outputfile="])
    except getopt.GetoptError:
        print usage
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print usage
            sys.exit()
        elif opt in ("-c", "--configfile"):
            configfile = arg
        elif opt in ("-f", "--inputfile"):
            inputfile = arg
        elif opt in ("-d", "--display"):
            display = True
        elif opt in ("-o", "--outputfile"):
            outputfile = arg
                        
    if(len(configfile)==0 or len(inputfile)==0):
        print usage
        sys.exit(2)
   
    size_result = Popen(["identify", inputfile], stdout=PIPE).communicate()[0]
    ( size_result_head, size_result_middle, size_result_tail ) = size_result.partition(inputfile+" ")
    ( size_result_head, size_result_middle, size_result_tail ) = size_result_tail.partition(" ")
    ( size_result, size_result_middle, size_result_tail ) = size_result_tail.partition(" ")
   
    (size_result_width, size_result_middle, size_result_height) = size_result.partition("x")
   
    #print 'Size: ' + size_result

    output_url = Popen(["./replace_content", inputfile, configfile], stdout=PIPE).communicate()[0]
    
    #print 'URL: ' + output_url
    
    if(len(output_url)>0):
        first_result = ''
        distortion = ''

        for url in string.split(output_url, "\n"):
            t = threading.Thread(target=read_url, args=([url]))
            t.start()
            thread_queue.put(t)
        
        while( not thread_queue.empty() ):
            t = thread_queue.get()
            t.join()
            
        while(not image_queue.empty()):
            result_image = image_queue.get()
            new_size_result = Popen(["identify", "-"], stdout=PIPE, stdin=PIPE).communicate(input=result_image)[0]

            ( new_size_result_head, new_size_result_middle, new_size_result_tail ) = new_size_result.partition(" ")
            ( new_size_result_head, new_size_result_middle, new_size_result_tail ) = new_size_result_tail.partition(" ")
            ( new_size_result, new_size_result_middle, new_size_result_tail ) = new_size_result_tail.partition(" ")
            
            ( new_size_result_width, new_size_result_middle, new_size_result_height ) = new_size_result.partition("x")
            
            if(len(first_result)==0):
                first_result = result_image
            
            if(len(size_result_width)>0 and len(new_size_result_width)>0 and len(size_result_height)>0 and len(new_size_result_height)>0):
                a = round(float(size_result_width)/float(new_size_result_width),2)
                b = round(float(size_result_height)/float(new_size_result_height),2)
                
                if(a==b):
                    distortion = Popen(["convert", "-", "-resize", size_result, "-"], stdout=PIPE, stdin=PIPE).communicate(input=result_image)[0]
                    if(display and len(distortion)>0):
                        Popen(["display"], stdin=PIPE).communicate(input=distortion)[0]
                    if(len(outputfile)>0 and len(distortion)>0):
                        f = open(outputfile, "w")
                        f.write(distortion)    
                        f.close()
                    break
                    
        if(len(distortion)==0 and len(first_result)>0):
            distortion = Popen(["convert", "-", "-resize", size_result, "-"], stdout=PIPE, stdin=PIPE).communicate(input=first_result)[0]
            if(display and len(distortion)>0):
                Popen(["display"], stdin=PIPE).communicate(input=distortion)[0]
            if(len(outputfile)>0 and len(distortion)>0):
                f = open(outputfile, "w")
                f.write(distortion)
                f.close()

def read_url(url):
    result_image = Popen(["wget", url, "-O", "-"], stdout=PIPE).communicate()[0]
    image_queue.put(result_image)

if __name__ == "__main__":
   main(sys.argv[1:])
