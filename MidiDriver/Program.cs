using Melanchall.DryWetMidi.Core;
using Melanchall.DryWetMidi.Multimedia;
using Melanchall.DryWetMidi.Common;
using System;
using System.IO.Ports;
using System.Threading;
using System.Collections;
using Melanchall.DryWetMidi.MusicTheory;

namespace MidiDriver;

class Program
{
    static SerialPort serialPort = new("COM4", 9600);

    static void Main(string[] args)
    {
        serialPort.Open();
        using var midiOut = OutputDevice.GetByName("arduino");
        midiOut.PrepareForEventsSending();
        byte[] packet = new byte[5];
        int currentIndex = 0;
        short lastData = 0;
        while (true)
        {
            if (serialPort.BytesToRead > 0)
            {
                byte nextByte = (byte)serialPort.ReadByte();
                if (nextByte == 0xFF) {
                    currentIndex = 0;
                    packet[currentIndex] = nextByte;
                    currentIndex++;
                    continue;
                }
                packet[currentIndex] = nextByte;
                currentIndex++;
                if (currentIndex == 5) {
                    short data = BitConverter.ToInt16([packet[2], packet[1]], 0);
                    BitArray bits = new(BitConverter.GetBytes(data));
                    BitArray lastBits = new(BitConverter.GetBytes(lastData));
                    byte octave = packet[3];
                    for (int i = 0; i < bits.Length; i++) {
                        byte note = (byte)(Note.Get(NoteName.C, octave - 1).NoteNumber + i);
                        if (note > 127) {
                            continue;
                        }
                        if (bits[i] != lastBits[i]) {
                            if (bits[i]) {
                                midiOut.SendEvent(new NoteOnEvent(new SevenBitNumber(note), new SevenBitNumber(100)));
                            } else {
                                midiOut.SendEvent(new NoteOffEvent(new SevenBitNumber(note), new SevenBitNumber(100)));
                            }
                        }
                    }
                    packet = new byte[5];
                    lastData = data;
                    currentIndex = 0;
                }
            }
        }
    }
}
