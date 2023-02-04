from BADDudeClient import BADDudeClient
from BADDudeDefines import *
from enum import Enum
from elftools.elf.elffile import ELFFile


class BADDudeCommand(Enum):
    BADDUDE_CMD_Flash = "f"
    BADDUDE_CMD_DumpFlash = "d"
    BADDUDE_CMD_Reset = "r"
    BADDUDE_CMD_RunCustomProgram = "c"
    BADDUDE_CMD_Quit = "q"
    BADDUDE_CMD_HELP = "h"

    def help():
        print("Command Help List:")
        print("f: Flash")
        print("d: Dump Flash")
        print("r: Reset")
        print("c: Run Custom Program")
        print("q: Quit")

    @classmethod
    def _missing_(cls, value):
        return BADDudeCommand.BADDUDE_CMD_Quit


def pollNextCommand() -> BADDudeCommand:
    print("\nSelect a command (h for help)")
    command = input("> ")
    print()

    return BADDudeCommand(command)


def manageCommand(command: BADDudeCommand, client: BADDudeClient):
    match command:
        case BADDudeCommand.BADDUDE_CMD_Flash:
            with open("main", 'rb') as f:
                elfFile = ELFFile(f)
                programData = extractFlashData(elfFile)

            print(
                "Program size:",
                str(len(programData)) + "/" + str(ATTINY_MEMORY_FLASH_SIZE),
                "bytes",
                "(" + str(round(len(programData)/ATTINY_MEMORY_FLASH_SIZE*100)) + "%)"
            )

            if (len(programData) > ATTINY_MEMORY_FLASH_SIZE):
                print("Program too large")
                return

            client.tpiEnter()
            print("Entered TPI mode")
            client.flashPrime()
            print("Primed flash")
            client.flashErase()
            print("Erased flash")

            # Write the program data
            client.flashWrite(
                programData,
                lambda progress:
                    print("Flashing progress:",
                          str(round(progress*100)) + "%",
                          end="\r"
                          )
            )
            print()

            # Verify the program data
            def verifyFlashData(startAddr: int, readFlashData: bytes):
                flashImage = \
                    programData + \
                    bytes(
                        bytearray(
                            [0xFF]*(ATTINY_MEMORY_FLASH_SIZE -
                                    len(programData))
                        )
                    )

                flashSpaceAddr = startAddr - ATTINY_MEMORY_FLASH_ADDR

                if (readFlashData != flashImage[flashSpaceAddr:flashSpaceAddr+len(readFlashData)]):
                    print("Verification failed at address", hex(startAddr))
                    print(readFlashData.hex(sep=" ", bytes_per_sep=1))
                    print(flashImage[flashSpaceAddr:flashSpaceAddr +
                          len(readFlashData)].hex(sep=" ", bytes_per_sep=1))
                    exit(1)
                else:
                    print(
                        "Verifying:",
                        str(round((flashSpaceAddr+BADDUDE_MAX_CHUNK_SIZE) /
                            ATTINY_MEMORY_FLASH_SIZE*100)) + "%",
                        end="\r"
                    )

            client.flashDump(verifyFlashData)
            print()

            client.tpiExit()
            print("Exited TPI mode")
        case BADDudeCommand.BADDUDE_CMD_DumpFlash:
            client.tpiEnter()
            print("Entered TPI mode")
            client.flashPrime()
            print("Primed flash")

            def printFlashData(startAddr: int, flashData: bytes):
                columnWidth = 16
                for i in range(int(BADDUDE_MAX_CHUNK_SIZE/columnWidth)):
                    rowFlashData = flashData[i * columnWidth:(i+1)*columnWidth]
                    print(
                        hex(startAddr + i*columnWidth) + ":",
                        rowFlashData.hex(sep=" ", bytes_per_sep=1)
                    )

            print("Flash Dump:")
            client.flashDump(printFlashData)
            client.tpiExit()
            print("Exited TPI mode")
        case BADDudeCommand.BADDUDE_CMD_Reset:
            client.attinyReset()
            print("ATtiny reset")
        case BADDudeCommand.BADDUDE_CMD_RunCustomProgram:
            print("not implemented yet")
        case BADDudeCommand.BADDUDE_CMD_HELP:
            BADDudeCommand.help()


def extractFlashData(elfFile: ELFFile) -> bytes:
    for segment in elfFile.iter_segments():
        return segment.data()


def selectPort(portList: list) -> str:
    selectedPort = None

    if (len(portList) == 0):
        print("No ports found")
        exit(1)
    elif (len(portList) == 1):
        selectedPort = portList[0]
    else:
        print("Select a port:")
        for i in range(len(portList)):
            print(str(i) + ": " + portList[i])
        selectedPort = portList[int(input("> "))]

    return selectedPort


if __name__ == "__main__":
    client = BADDudeClient()

    portList = client.getPortList()
    selectedPort = selectPort(portList)
    client.begin(selectedPort)
    print("Successfully Connected to " + selectedPort)

    while True:
        nextCommand = pollNextCommand()
        if (nextCommand == BADDudeCommand.BADDUDE_CMD_Quit):
            break

        manageCommand(nextCommand, client)

    client.end()
    print("Disconnected from " + selectedPort)