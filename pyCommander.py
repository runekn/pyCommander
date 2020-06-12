import ctypes as ct
import time
import os

class Commander():
	def __init__(self):
		self.libcomm = ct.WinDLL("Commander.dll")
		
		self.libcomm.begin.argtypes = [ct.c_char_p, ct.c_ulong]
		self.libcomm.begin.restype = ct.c_bool
		
		self.libcomm.ReadMsgs.argtypes = None
		self.libcomm.ReadMsgs.restype = ct.c_int
	
	def begin(self, port, baudrate):
		self.libcomm.begin(port, baudrate)
	
	def read_messages(self):
		self.libcomm.ReadMsgs()
	
	def _get_button(self, button):
		bit_location = ct.c_uint8.in_dll(self.libcomm, button).value
		buttons = ct.c_uint8.in_dll(self.libcomm, "buttons").value
		return (buttons & (1 << bit_location)) != 0
	
	@property
	def walkV(self):
		return ct.c_int8.in_dll(self.libcomm, "walkV").value
	@property
	def walkH(self):
		return ct.c_int8.in_dll(self.libcomm, "walkH").value
	@property
	def lookV(self):
		return ct.c_int8.in_dll(self.libcomm, "lookV").value
	@property
	def lookH(self):
		return ct.c_int8.in_dll(self.libcomm, "lookH").value
	@property
	def BUT_R1(self):
		return self._get_button("BUT_R1")
	@property
	def BUT_R2(self):
		return self._get_button("BUT_R2")
	@property
	def BUT_R3(self):
		return self._get_button("BUT_R3")
	@property
	def BUT_L4(self):
		return self._get_button("BUT_L4")
	@property
	def BUT_L5(self):
		return self._get_button("BUT_L5")
	@property
	def BUT_L6(self):
		return self._get_button("BUT_L6")
	@property
	def BUT_RT(self):
		return self._get_button("BUT_RT")
	@property
	def BUT_LT(self):
		return self._get_button("BUT_LT")

if __name__ == "__main__":
	comm = Commander()
	comm.begin(b"COM6", 38400)
	while(True):
		comm.read_messages()
		print("\r", comm.walkV, comm.walkH, comm.lookV, comm.lookH, comm.BUT_R1, comm.BUT_R2, comm.BUT_R3, comm.BUT_L4, comm.BUT_L5, comm.BUT_L6, comm.BUT_RT, comm.BUT_LT, end="")
		time.sleep(0.01)