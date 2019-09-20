from __future__ import division

import re
import sys
import os
import serial

from google.cloud import speech
from google.cloud.speech import enums
from google.cloud.speech import types
import pyaudio
from six.moves import queue

credential_path = "D:\Masaüstü\Thesis\My First Project-aec07b5ab79c.json"
os.environ['GOOGLE_APPLICATION_CREDENTIALS'] = credential_path

# Audio recording parameters
RATE = 10000
CHUNK = int(RATE / 10)  # 100ms


# binary çevirme kaldırdım
"""
binFromX = ""
binFromY = ""
binToX = ""
binToY = ""


letters ={
            "a": "000", "b": "001", "c": "010", "d": "011",
            "e": "100", "f": "101", "g": "110", "h": "111",
            }
numbers ={
            "1": "000", "2": "001", "3": "010", "4": "011",
            "5": "100", "6": "101", "7": "110", "8": "111",
            }

binFromX , binFromY = letters[xFrom],numbers[yFrom]
binToX , binToY = letters[xTo],numbers[yTo]
"""


moveLetters =  ['adana','bursa','ceyhan','denizli','edirne','fransa','giresun','hatay']
numberLetters = ['1','2','3','4','5','6','7','8']
xFrom = ""
yFrom = ""
xTo = ""
yTo = ""



class MicrophoneStream(object):
    """Opens a recording stream as a generator yielding the audio chunks."""
    def __init__(self, rate, chunk):
        self._rate = rate
        self._chunk = chunk

        # Create a thread-safe buffer of audio data
        self._buff = queue.Queue()
        self.closed = True

    def __enter__(self):
        self._audio_interface = pyaudio.PyAudio()
        self._audio_stream = self._audio_interface.open(
            format=pyaudio.paInt16,
            # The API currently only supports 1-channel (mono) audio
            # https://goo.gl/z757pE
            channels=1, rate=self._rate,
            input=True, frames_per_buffer=self._chunk,
            # Run the audio stream asynchronously to fill the buffer object.
            # This is necessary so that the input device's buffer doesn't
            # overflow while the calling thread makes network requests, etc.
            stream_callback=self._fill_buffer,
        )

        self.closed = False

        return self

    def __exit__(self, type, value, traceback):
        self._audio_stream.stop_stream()
        self._audio_stream.close()
        self.closed = True
        # Signal the generator to terminate so that the client's
        # streaming_recognize method will not block the process termination.
        self._buff.put(None)
        self._audio_interface.terminate()

    def _fill_buffer(self, in_data, frame_count, time_info, status_flags):
        """Continuously collect data from the audio stream, into the buffer."""
        self._buff.put(in_data)
        return None, pyaudio.paContinue

    def generator(self):
        while not self.closed:
            # Use a blocking get() to ensure there's at least one chunk of
            # data, and stop iteration if the chunk is None, indicating the
            # end of the audio stream.
            chunk = self._buff.get()
            if chunk is None:
                return
            data = [chunk]

            # Now consume whatever other data's still buffered.
            while True:
                try:
                    chunk = self._buff.get(block=False)
                    if chunk is None:
                        return
                    data.append(chunk)
                except queue.Empty:
                    break

            yield b''.join(data)
            
            
def checkTheStreamToFindMove(textStream):
    global letters,numbers,moveLetters,numberLetters,xFrom,yFrom,xTo,yTo
    
    if textStream[0] == " ":
        textStream=textStream[1:]
    
    textStream=textStream.lower()
    strAll = textStream.split(" ")
    print ("Check Reached ** " + textStream + " **\r")
    if "ve" in strAll[2]:
        if strAll[0] in moveLetters and strAll[3] in moveLetters:
            if strAll[1] in numberLetters and strAll[4] in numberLetters:
                xFrom, yFrom  = strAll[0][0], strAll[1][0]
                xTo , yTo = strAll[3][0], strAll[4][0]
                sendValidMoveToArduino(xFrom+yFrom+xTo+yTo)
                print (xFrom + "  " + yFrom)
                print (xTo + "  " + yTo)
    

def sendValidMoveToArduino(sendData):
    ser = serial.Serial ("/dev/ttyS0",9600,timeout=1)       
    ser.write(sendData)
    ser.close()

    


def listen_print_loop(responses):

    
    num_chars_printed = 0
    for response in responses:
        if not response.results:
            continue

        # The `results` list is consecutive. For streaming, we only care about
        # the first result being considered, since once it's `is_final`, it
        # moves on to considering the next utterance.
        result = response.results[0]
        if not result.alternatives:
            continue

        # Display the transcription of the top alternative.
        transcript = result.alternatives[0].transcript

        # Display interim results, but with a carriage return at the end of the
        # line, so subsequent lines will overwrite them.
        #
        # If the previous result was longer than this one, we need to print
        # some extra spaces to overwrite the previous result
        overwrite_chars = ' ' * (num_chars_printed - len(transcript))

        if not result.is_final:
            sys.stdout.write(transcript + overwrite_chars + '\r')
            sys.stdout.flush()

            num_chars_printed = len(transcript)

        else:
            print(transcript + overwrite_chars)
            

            # Exit recognition if any of the transcribed phrases could be
            # one of our keywords.
            if re.search(r'\b(Çıkış|exit|çıkış)\b', transcript, re.I):
                print('Exiting..')
                return True
            
            checkTheStreamToFindMove(transcript)
            num_chars_printed = 0
            return False


def main():
    # See http://g.co/cloud/speech/docs/languages
    # for a list of supported languages.
    language_code = 'tr-TR'  # a BCP-47 language tag
    
    while(True==True):
        client = speech.SpeechClient()
        config = types.RecognitionConfig(
            encoding=enums.RecognitionConfig.AudioEncoding.LINEAR16,
            sample_rate_hertz=RATE,
            language_code=language_code)
        streaming_config = types.StreamingRecognitionConfig(
            config=config,
            interim_results=True)
    
        with MicrophoneStream(RATE, CHUNK) as stream:
            audio_generator = stream.generator()
            requests = (types.StreamingRecognizeRequest(audio_content=content)
                        for content in audio_generator)
    
            responses = client.streaming_recognize(streaming_config, requests)
    
            # Now, put the transcription responses to use.
            try:
                if listen_print_loop(responses) == True:
                    exit()
            except Exception as exception:
                print("")


if __name__ == '__main__':
    main()