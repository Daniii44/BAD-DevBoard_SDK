from BADDudeDefines import *
import serial
import serial.tools.list_ports
from typing import Callable
import glob

BADDUDE_BAUDRATE = 115200

BADDUDE_START_KEY = 0x89ABCDEF
BADDUDE_CPCPLT_KEY = 0xC0C1C2C3


BADDUDE_LL_CMD_NOP = 0x00
BADDUDE_LL_CMD_ACK = 0x01
BADDUDE_LL_CMD_NACK = 0x02
BADDUDE_LL_CMD_CPLT = 0x03
BADDUDE_LL_CMD_NEXTCHUNK = 0x04
BADDUDE_LL_CMD_PING = 0x05

BADDUDE_LL_CMD_TPIENTER = 0x10
BADDUDE_LL_CMD_TPIEXIT = 0x11

BADDUDE_LL_CMD_FPRIME = 0x20
BADDUDE_LL_CMD_FERASE = 0x21
BADDUDE_LL_CMD_FWRITE = 0x22
BADDUDE_LL_CMD_FWPROG = 0x23
BADDUDE_LL_CMD_FREAD = 0x2A

BADDUDE_CMD_ATRESET = 0x30

BADDUDE_CMD_CPCNT = 0x40
BADDUDE_CMD_CPTITLE = 0x41
BADDUDE_CMD_CPRUN = 0x42


class BADDudeClient:
    _ser: serial.Serial

    def getPortList(self) -> list:
        ports = glob.glob('/dev/cu.usb*')
        return ports

    def begin(self, port: str):
        self._ser = serial.Serial(port, BADDUDE_BAUDRATE)
        self._ser.timeout = None
        self._ser.rts = True
        self._ser.dtr = False
        self._pollStartKey()

    def end(self):
        self._ser.close()

    def ping(self, verbose=True):
        self._writeInt8(BADDUDE_LL_CMD_PING)
        self._pollAck()

    def attinyReset(self):
        self._writeInt8(BADDUDE_CMD_ATRESET)
        self._pollAck()

    def tpiEnter(self):
        self._writeInt8(BADDUDE_LL_CMD_TPIENTER)
        self._pollAck()

    def tpiExit(self):
        self._writeInt8(BADDUDE_LL_CMD_TPIEXIT)
        self._pollAck()

    def flashPrime(self):
        self._writeInt8(BADDUDE_LL_CMD_FPRIME)
        self._pollAck()

    def flashErase(self):
        self._writeInt8(BADDUDE_LL_CMD_FERASE)
        self._pollAck()

    def flashWrite(self, bytes: bytes, progressCallback: Callable[[float], None]):
        self._writeInt8(BADDUDE_LL_CMD_FWRITE)
        self._writeInt16(len(bytes))
        self._writeBytes(bytes, chunked=True)

        self._pollAck()

        while True:
            if (self._readInt8() == BADDUDE_LL_CMD_FWPROG):
                progress = self._readInt8()
                progressCallback(progress/255)
                if progress == 0xFF:
                    return

    def flashDump(self, progressCallback: Callable[[int, bytes], None]):
        self._writeInt8(BADDUDE_LL_CMD_FREAD)
        self._pollAck()
        for addr in range(0x4000, 0x4800, BADDUDE_MAX_CHUNK_SIZE):
            bytes = self._readBytes(BADDUDE_MAX_CHUNK_SIZE)
            self._writeInt8(BADDUDE_LL_CMD_NEXTCHUNK)
            progressCallback(addr, bytes)

    def getCustomProgramCount(self) -> int:
        self._writeInt8(BADDUDE_CMD_CPCNT)
        self._pollAck()
        return self._readInt8()

    def getCustomProgramTitle(self, programID: int) -> str:
        self._writeInt8(BADDUDE_CMD_CPTITLE)
        self._writeInt8(programID)
        self._pollAck()

        title = ""
        while True:
            receivedByte = self._readBytes(1)
            if receivedByte == b'\x00':
                return title
            else:
                title += receivedByte.decode("utf-8")

    def runCustomProgram(self, programID: int, traceCallback: Callable[[str], None]):
        self._writeInt8(BADDUDE_CMD_CPRUN)
        self._writeInt8(programID)
        self._pollAck()

        remainingKey = BADDUDE_CPCPLT_KEY
        while True:
            receivedByte = self._readInt8()
            if receivedByte == remainingKey & 0xFF:
                remainingKey = remainingKey >> 8
                if remainingKey == 0:
                    return
            else:
                traceCallback(bytes([receivedByte]).decode("utf-8"))

    def _pollStartKey(self):
        remainingKey = BADDUDE_START_KEY
        while True:
            receivedByte = self._readInt8()
            if receivedByte == (remainingKey & 0xFF):
                remainingKey = remainingKey >> 8
                if remainingKey == 0:
                    return
            else:
                remainingKey = BADDUDE_START_KEY

    def _pollAck(self):
        while True:
            receivedByte = self._readInt8()
            if receivedByte == BADDUDE_LL_CMD_ACK:
                return
            elif receivedByte == BADDUDE_LL_CMD_NACK:
                raise Exception("NACK received")

    # Low level functions
    def _writeInt8(self, int8: int):
        self._ser.write(int8.to_bytes(1, byteorder='little'))

    def _writeInt16(self, int16: int):
        self._ser.write(int16.to_bytes(2, byteorder='little'))

    def _writeBytes(self, bytes: bytes, chunked: bool):
        if not chunked:
            self._ser.write(bytes)
        else:
            for i in range(0, len(bytes), BADDUDE_MAX_CHUNK_SIZE):
                chunk = bytes[i:i+BADDUDE_MAX_CHUNK_SIZE]
                self._writeBytes(chunk, chunked=False)
                if self._readInt8() != BADDUDE_LL_CMD_NEXTCHUNK:
                    raise Exception("Unexpected response from device")

    def _readInt8(self, ) -> int:
        return int.from_bytes(self._ser.read(1), byteorder='little')

    def _readBytes(self, length: int) -> bytes:
        return self._ser.read(length)
