import sys
import argparse
from bluepy import btle
 
HRM_DEV_LOG_SERVICE_UUID = "631d0001-cbb8-4619-af07-48060160121c"
HRM_DEV_LOG_PRINT_CHAR_UUID = "631d0002-cbb8-4619-af07-48060160121c"
HRM_DEV_LOG_START_CHAR_UUID = "631d0003-cbb8-4619-af07-48060160121c"

fmtMillisStart = 0
fmtMillisStop = 4
fmtGyroXstart = 4
fmtGyroXstop = 6
fmtGyroYstart = 6
fmtGyroYstop = 8
fmtGyroZstart = 8
fmtGyroZstop = 10
fmtAccXstart = 10
fmtAccXstop = 12
fmtAccYstart = 12
fmtAccYstop = 14
fmtAccZstart = 14
fmtAccZstop = 16
fmtPitchStart = 16
fmtPitchStop = 18
fmtRollStart = 18
fmtRollStop = 20
fmtYawStart = 20
fmtYawStop = 22
fmtValStart = 22
fmtValStop = 24
fmtBpmStart = 24
fmtBpmStop = 26
fmtExtPos = 26

class MyDelegate(btle.DefaultDelegate):
    def __init__(self, params):
        self.tgt = params
        btle.DefaultDelegate.__init__(self)

    def handleNotification(self, cHandle, data):
        if self.tgt == cHandle:
            # print(data.decode(), end="")
            millis = int.from_bytes(data[fmtMillisStart:fmtMillisStop], byteorder='little', signed=False)
            gyroX = (int.from_bytes(data[fmtGyroXstart:fmtGyroXstop], byteorder='little', signed=True)) / 100
            gyroY = (int.from_bytes(data[fmtGyroYstart:fmtGyroYstop], byteorder='little', signed=True)) / 100
            gyroZ = (int.from_bytes(data[fmtGyroZstart:fmtGyroZstop], byteorder='little', signed=True)) / 100
            accX = int.from_bytes(data[fmtAccXstart:fmtAccXstop], byteorder='little', signed=True)
            accY = int.from_bytes(data[fmtAccYstart:fmtAccYstop], byteorder='little', signed=True)
            accZ = int.from_bytes(data[fmtAccZstart:fmtAccZstop], byteorder='little', signed=True)
            pitch = int.from_bytes(data[fmtPitchStart:fmtPitchStop], byteorder='little', signed=True) / 100
            roll = int.from_bytes(data[fmtRollStart:fmtRollStop], byteorder='little', signed=True) / 100
            yaw = int.from_bytes(data[fmtYawStart:fmtYawStop], byteorder='little', signed=True) / 100
            val = int.from_bytes(data[fmtValStart:fmtValStop], byteorder='little', signed=True)
            bpm = int.from_bytes(data[fmtBpmStart:fmtBpmStop], byteorder='little', signed=True)
            peakP = 'false'
            if (data[fmtExtPos] & 0x01) == 0x01:
                peakP = 'true'
            peakN = 'false'
            if (data[fmtExtPos] & 0x02) == 0x02:
                peakN = 'true'
            print(f'{{"millis":{millis}, "gyroX":{gyroX}, "gyroY":{gyroY}, "gyroZ":{gyroZ}, "accX":{accX}, "accY":{accY}, "accZ":{accZ}, "pitch":{pitch}, "roll":{roll}, "yaw":{yaw}, "val":{val}, "bpm":{bpm},"peakP":{peakP}, "peakN":{peakN}}}')
 

def main(deviceAddr):
    peri = btle.Peripheral()
    # peri.connect(deviceAddr, btle.ADDR_TYPE_RANDOM)
    peri.connect(deviceAddr)
    peri.setMTU(30)
    printC=peri.getCharacteristics(uuid=HRM_DEV_LOG_PRINT_CHAR_UUID)[0]
    peri.setDelegate(MyDelegate(printC.getHandle()))
    
    startC=peri.getCharacteristics(uuid=HRM_DEV_LOG_START_CHAR_UUID)[0]
    startC.write(b'\x06')
    
    while True:
        if peri.waitForNotifications(1.0):
            # handleNotification() was called
            continue
    
        print ("Waiting...")
        # Perhaps do something else here

    #peri.disconnect()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('-a','--addr', nargs=1, required=True, help='device addr')
    args = parser.parse_args()
    main(deviceAddr=args.addr[0])
