import speech_recognition as sr
import io
import os
import time
  
#enter the name of usb microphone that you found
#using lsusb
#the following name is only used as an example
#mic_name = "USB Device#= 0x46d:0x825: Audio (hw:1, 0)"
#Sample rate is how often values are recorded
sample_rate = 48000
#Chunk is like a buffer. It stores 2048 samples (bytes of data)
#here.
#it is advisable to use powers of 2 such as 1024 or 2048
chunk_size = 2048
#Initialize the recognizer
r = sr.Recognizer()
f_sentence= open("test.txt","w+")
with sr.Microphone(device_index = 0, sample_rate = sample_rate, chunk_size = chunk_size) as source:
    #wait for a second to let the recognizer adjust the
    #energy threshold based on the surrounding noise level
    r.adjust_for_ambient_noise(source)
    print ("Say Something")
    #listens for the user's input
    audio = r.listen(source)
          
    try:
        
        text = r.recognize_google(audio)

        print ("I will change the sentence into script")
        if (text[0:1] == "is"):
            text[1] = "f"
        text = text.replace(" then", ", then")
        text = text.replace("and, then", ", and then")
        print ("you said: " + text)
        
        f_sentence.write(text)
        f_sentence.close()
        cmd = "./batch.sh"
        returned_value = os.system(cmd)
        time.sleep(3)
        f_script= open("prediction.txt","r")
        temp_script = f_script.readline()
        f_script.close()
        print(temp_script)
        
      
    #error occurs when google could not understand what was said
      
    except sr.UnknownValueError:
        print("Google Speech Recognition could not understand audio")
      
    except sr.RequestError as e:
        print("Could not request results from Google Speech Recognition service; {0}".format(e))

    
