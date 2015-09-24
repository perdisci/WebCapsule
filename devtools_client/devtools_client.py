import websocket
import thread
import json
import sys
import logging
import shlex
from devtools_common import Command, MessageOffloader, DevtoolsApp

class ClientTermination(Exception):
	pass

class DevtoolsClient(DevtoolsApp):
	def __init__(self):
		super(DevtoolsClient, self).__init__()
		self.offloader = MessageOffloader()
	
	#override
	def handleUnknown(self, mess):
		self.writeMessage("Received: " + mess)
	
	#override
	def handleEvent(self, mess):
		if mess["method"] == "Forensics.recordingAdd":
			self.offloader.messages.put(mess)
		else:
			self.writeMessage(json.dumps(mess, separators=(',', ':')))
	
	#override
	def handleResponse(self, idval):
		cmd = self.cmdHistory[idval]
		if (cmd.cmd == Command.DumpRecording or cmd.cmd == Command.DumpReplayInfo) and "file" in cmd.params:
			with open(cmd.params["file"], 'w') as outfile:
				outfile.write(json.dumps(cmd.resp["result"], sort_keys=True,
										indent=4, separators=(',', ':')))
				outfile.write("\n")
			self.writeMessage("Saved to " + cmd.params["file"]);
		else:
			self.writeMessage("Received: " + json.dumps(cmd.resp, separators=(',', ':')))
	
	#override
	def onError(self, ws, error):
		self.writeMessage(error)
	
	#override
	def onClose(self, ws):
		self.offloader.stop()
		self.offloader.join()
		print "Exiting"
	
	#override    
	def onOpen(self, ws):
		def run(*args):
			self.offloader.start()
			# Input processing loop
			while True:
				self.writePrompt()
				user_input = sys.stdin.readline().strip()
				try:
					if not self.handleClientCommand(user_input):
						cmd = self.sendCommand(self.parseCommand(user_input))
						print "Sent: ", cmd
				except ClientTermination:
					break
				except Exception as e:
					print "Error: ", e
			self.stop()

		thread.start_new_thread(run, ())
		
	def handleClientCommand(self, cmd_str):
		retval = False
		if cmd_str == "":
			retval = True
		elif cmd_str == "exit":
			raise ClientTermination
		elif cmd_str == "help":
			retval = True
			print "Available commands: ", Command.cmds
		return retval
	
	#override    
	def pickUrl(self, data):
		selection = -1
		print "Select the page. (Enter the page's number)"
		while selection < 0 or selection > len(data) - 1:
			self.writePageChoices(data)
			self.writePrompt()
			user_input = sys.stdin.readline().strip()
			selection = int(user_input)
		return data[selection]["webSocketDebuggerUrl"]

	def writePageChoices(self, data):
		count = 0
		for d in data:
			print "(%i) Title: %s Url: %s ID: %s" % (count, d["title"], d["url"], d["id"])
			count += 1
	
	def writeMessage(self, message):	
		sys.stdout.write("\n" + message + "\n")
		sys.stdout.flush()
		self.writePrompt()

	def writePrompt(self, inval=None):
		prompt = ':> '
		if inval is not None:
			prompt += inval
		sys.stdout.write(prompt);
		sys.stdout.flush()
		
	def parseCommand(self, cmd_str):
		cmd_str_comps = cmd_str.split()
		cmd = cmd_str_comps[0]
		params = self.parseParams(cmd_str)
		return Command(cmd, params)

	def parseParams(self, cmd_str):
		retval = {};
		cmd_str_comps = shlex.split(cmd_str)
		if len(cmd_str_comps) > 1 :
			cmd = cmd_str_comps[0]
			if cmd == Command.StartReplay:
				retval["speed"] = float(cmd_str_comps[1])
			elif cmd == Command.DumpRecording or cmd == Command.DumpReplayInfo or cmd == Command.LoadRecording:
				retval["file"] = cmd_str_comps[1]
				
		return retval;
		
def main():
	logging.raiseExceptions = 0
	websocket.enableTrace(False)
	if len(sys.argv) > 1:
		app = DevtoolsClient()
		app.run(sys.argv[1])
	else :
		print "Usage: %s <devtools_url>" % (sys.argv[0])
	
if __name__ == "__main__":
	main()