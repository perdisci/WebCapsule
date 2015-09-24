'''
Created on Mar 12, 2015

@author: cjneasbi
'''

import json
import time
import multiprocessing
import urllib2
import websocket

#generic websocket application for interacting with devtools
class DevtoolsApp(multiprocessing.Process):
    def __init__(self):
        super(DevtoolsApp, self).__init__()
        self.cmdHistory = dict()
    
    #override
    def handleUnknown(self, mess):
        pass
    
    #override
    def handleEvent(self, mess):
        pass
    
    #override
    def handleResponse(self, idval):
        pass

    #override
    def onError(self, ws, error):
        pass

    #override
    def onClose(self, ws):
        pass

    #override    
    def onOpen(self, ws):
        pass
    
    #override    
    def pickUrl(self, data):
        pass
    
    def onMessage(self, ws, message):
        mess = json.loads(message)
        if "id" in mess:
            idval = self.pairResponse(mess)
            self.handleResponse(idval)
        elif "method" in mess:
            self.handleEvent(mess)
        else:
            self.handleUnknown(mess)
            
    def pairResponse(self, message):
        idval = message["id"]
        if idval in self.cmdHistory:
            self.cmdHistory[idval].setResponse(message)
            return idval
        else:
            raise Exception("Could not pair command response to executed command, id: " + idval)

    def sendCommand(self, cmd):
        self.cmdHistory[cmd.id] = cmd
        self.ws.send(cmd.commandString())
        return cmd.cmd
        
    def stop(self):
        self.ws.close()
        
    def selectUrl(self, url):
        resp = urllib2.urlopen(url)
        lines = "\n".join(resp.readlines())
        data = json.loads(lines)
        return self.pickUrl(data)
        
    def run(self, connectUrl):
        wsurl = self.selectUrl(connectUrl)
        self.ws = websocket.WebSocketApp(wsurl,
            on_open = self.onOpen,
            on_message=self.onMessage,
            on_error=self.onError,
            on_close=self.onClose)
        self.ws.run_forever()


#begin command factory methods

def createStartReplay(speed=None):
    params = {}
    if speed:
        params["speed"] = speed
    return Command(Command.StartReplay, params)

def createLoadRecording(dumpFile):
    params = {}
    params["file"] = dumpFile
    return Command(Command.LoadRecording, params)

def createDumpReplayInfo(outFile):
    params = {}
    params["file"] = outFile
    return Command(Command.DumpReplayInfo, params)

#end command factory methods

#Forensics devtools commands 
class Command(object):
    
    # Command const
    StartRecording = "StartRecording"
    StopRecording = "StopRecording"
    DumpRecording = "DumpRecording"
    DumpReplayInfo = "DumpReplayInfo"
    LoadRecording = "LoadRecording"
    StartReplay = "StartReplay"
    StopReplay = "StopReplay"
    GetState = "GetState"

    # valid commands list
    cmds = [StartRecording, StopRecording, DumpRecording, DumpReplayInfo, LoadRecording, StartReplay, StopReplay, GetState]

    # for request ids
    __id_counter = 0
    
    # command to method translation
    __remote_methods = {StartRecording : "Forensics.startRecording",
        StopRecording : "Forensics.stopRecording",
        DumpRecording : "Forensics.dumpRecording",
        DumpReplayInfo : "Forensics.dumpReplayInfo",
        LoadRecording : "Forensics.loadRecording",
        StartReplay : "Forensics.startReplay",
        StopReplay : "Forensics.stopReplay",
        GetState : "Forensics.getState"}

    def __init__(self, cmd, params={}):
        if cmd in self.cmds :
            self.cmd = cmd
            self.id = self.__class__.__id_counter
            self.__class__.__id_counter += 1
            self.params = params
            self.resp = None
        else :
            raise Exception("Not a valid command")

    def __repr__(self):
        return json.dumps(self.__genDict(), separators=(',', ':'))

    def __str__(self):
        return json.dumps(self.__genDict(), sort_keys=True,
            indent=4, separators=(',', ': '))

    def __genDict(self):
        retval = {"method": self.__remote_methods[self.cmd], "id": self.id}
        if self.params:
            retval["params"] = self.params
        return retval
    
    def commandString(self):
        retval = {"method": self.__remote_methods[self.cmd], "id": self.id}
        if self.params:
            if self.cmd == self.StartReplay:
                retval["params"] = self.params
            if self.cmd == self.LoadRecording:
                with open(self.params["file"], 'r') as content_file: 
                    retval["params"] = json.loads(content_file.read())
        return json.dumps(retval, separators=(',', ':'))

    def setResponse(self, message):
        if self.resp == None and message["id"] == self.id:
            self.resp = message

#generic message handling process    
class MessageHandler(multiprocessing.Process):
    def __init__(self):
        super(MessageHandler, self).__init__()
        self.messages = multiprocessing.JoinableQueue()
        
    def run(self):
        self.onStart()
        self.start_time = long(time.time() * 100)
        while True:
            mess = self.messages.get()
            if mess is None:
                self.onStop()    
                break
            self.onMessage(mess)
            
    def stop(self):
        self.messages.put(None)
            
    def onStart(self):
        pass

    def onMessage(self, message):
        pass
    
    def onStop(self):
        pass
    
#message handler process for offloading messages
class MessageOffloader(MessageHandler):
    def __init__(self):
        super(MessageOffloader, self).__init__()
        self.file_inited = False;
        
    def onStart(self):
        self.start_time = long(time.time() * 100)
        
    def onMessage(self, message):
        with open("./offloaded-data-" + str(self.start_time) + ".txt", 'a') as outfile:
            if not self.file_inited:
                outfile.write("[\n")
                self.file_inited = True
            else:
                outfile.write(",\n")
            outfile.write(json.dumps(message, sort_keys=True,
                                    indent=4, separators=(',', ':')))          
    def onStop(self):
        if self.file_inited:
            with open("./offloaded-data-" + str(self.start_time) + ".txt", 'a') as outfile:
                outfile.write("\n]")
        