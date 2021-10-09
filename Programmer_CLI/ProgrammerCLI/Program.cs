using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static ProgrammerCLI.Programmer_Actions;

namespace ProgrammerCLI
{
    public enum CommunicationMode
    {
        Serial, HTTP, NONE
    }

    public enum ArgAction
    {
        Read, Write, Reset, Help, Serial_List, None, PinConf_GET, PinConf_SET, Wifi_Set, Wifi_Get, Debug_ON, Debug_OFF, IP_Data
    }

    class Program
    {
        static void Main(string[] args)
        {
            ArgAction action = ArgAction.None;
            DeviceVector vector = new DeviceVector();

            for (int i = 0; i < args.Length; i++)
            {
                var item = args[i];
                if (item == "--upload" || item == "-u")
                {
                    action = ArgAction.Write;
                }
                if (item == "--read" || item == "-r")
                {
                    action = ArgAction.Read;
                }
                if (item == "--file" || item == "-f")
                {
                    vector.File = ReadArg(args, i + 1);
                }
                if (item == "--help")
                {
                    action = ArgAction.Help;
                }
                if (item == "--http" || item == "-h")
                {
                    vector.mode = CommunicationMode.HTTP;
                    vector.DeviceAddress = ReadArg(args, i + 1);
                }
                if (item == "--serial" || item == "-s")
                {
                    vector.mode = CommunicationMode.Serial;
                    vector.DeviceAddress = ReadArg(args, i + 1);
                }
                if (item == "--reset")
                {
                    action = ArgAction.Reset;
                }
                if (item == "--list-serial")
                {
                    action = ArgAction.Serial_List;
                }
                if (item == "--address")
                {
                    vector.StartAddress = Convert.ToInt32(ReadArg(args, i + 1));
                }
                if (item == "--count")
                {
                    vector.Count = Convert.ToInt32(ReadArg(args, i + 1));
                }
                if (item == "--pins")
                {
                    var op = ReadArg(args, i + 1);
                    if (op == "get")
                    {
                        action = ArgAction.PinConf_GET;
                    }else if (op == "set")
                    {
                        action = ArgAction.PinConf_SET;
                    }
                    else
                    {
                        Console.WriteLine("ERROR: Invalid pins parameters");
                        Program.Exit(1);
                    }
                }
                if (item == "--data")
                {
                    vector.Data = ReadArg(args, i + 1);
                }
                if (item == "--wificonf")
                {
                    var op = ReadArg(args, i + 1);
                    if (op == "get")
                    {
                        action = ArgAction.Wifi_Get;
                    }
                    else if (op == "set")
                    {
                        action = ArgAction.Wifi_Set;
                        vector.args["ssid"] = ReadArg(args, i + 2);
                        vector.args["pass"] = ReadArg(args, i + 3);
                    }
                    else if (op == "ap")
                    {
                        action = ArgAction.Wifi_Set;
                        vector.args["ap_ssid"] = ReadArg(args, i + 2);
                        vector.args["ap_pass"] = ReadArg(args, i + 3);
                    }
                    else
                    {
                        Console.WriteLine("ERROR: Invalid wificonf parameters");
                        Program.Exit(1);
                    }
                }
                if (item == "--debug")
                {
                    var op = ReadArg(args, i + 1);
                    if (op == "0")
                    {
                        action = ArgAction.Debug_OFF;
                    }
                    else if (op == "1")
                    {
                        action = ArgAction.Debug_ON;
                    }
                    else
                    {
                        Console.WriteLine("ERROR: Invalid debug parameters");
                        Program.Exit(1);
                    }
                }
                if (item == "--ip")
                {
                    action = ArgAction.IP_Data;
                }
            }

            if (action == ArgAction.Write) Programmer_Actions.Upload(vector);
            if (action == ArgAction.Read) Programmer_Actions.Read(vector);
            if (action == ArgAction.Reset) Programmer_Actions.Reset(vector);
            if (action == ArgAction.PinConf_GET) Programmer_Actions.PinConfGet(vector);
            if (action == ArgAction.PinConf_SET) Programmer_Actions.PinConfSet(vector);
            if (action == ArgAction.Wifi_Set) Programmer_Actions.WifiConfSet(vector);
            if (action == ArgAction.Wifi_Get) Programmer_Actions.WifiConfGet(vector);
            if (action == ArgAction.Debug_ON) Programmer_Actions.DebugMode(vector,true);
            if (action == ArgAction.Debug_OFF) Programmer_Actions.DebugMode(vector,false);
            if (action == ArgAction.IP_Data) Programmer_Actions.IP_Data(vector);

            if (action == ArgAction.Help)
            {
                PrintHelp();
                Exit(0);
            }

            if (action == ArgAction.Serial_List)
            {
                PrintSerials();
                Exit(0);
            }

            PrintHelp();
            Exit(1);
        }

        public static void PrintHelp()
        {
            Console.WriteLine("EEPROM Programmer:");
            Console.WriteLine("[--upload | -u]              - Upload mode");
            Console.WriteLine("[--read | -r]                - Read mode");
            Console.WriteLine("[--file | -f] <file>         - File to read/write");
            Console.WriteLine("[--http | -h] <address>      - HTTP comunication mode");
            Console.WriteLine("[--serial | -s] <port>       - Serial comunication mode");
            Console.WriteLine("--reset                      - Reset wifi configuration (serial only)");
            Console.WriteLine("--address <startaddress>     - Set start address");
            Console.WriteLine("--count <count>              - Set count");
            Console.WriteLine("--list-serial                - Prints all connected serial ports");
            Console.WriteLine("--pins [get|set]             - Gets or sets the pin mappings");
            Console.WriteLine("--data <data>                - Set data");
            Console.WriteLine("--wificonf set <ssid> <pass> - Set wifi configuration");
            Console.WriteLine("--wificonf ap  <ssid> <pass> - Set AP wifi configuration");
            Console.WriteLine("--wificonf get               - Get wifi configuration (serial only)");
            Console.WriteLine("--debug [0|1]                - Set debug mode (serial only)");
            Console.WriteLine("--ip                         - Get ip configuration (serial only)");
            Console.WriteLine("--help                       - This help menu");

        }

        public static void Exit(int code)
        {
            //Console.ReadKey();
            Environment.Exit(code);
        }

        public static void PrintSerials()
        {
            foreach (var item in SerialPort.GetPortNames())
            {
                Console.WriteLine(item);
            }
        }

        public static string ReadArg(string[] args, int i)
        {
            if (args.Length <= i)
            {
                Console.WriteLine("ERROR: Too few arguments");
                Program.Exit(1);
            }
            return args[i];
        }
    }
}
