using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace ProgrammerCLI
{
    class Programmer_Actions
    {
        public enum REQUEST_TYPE
        {
            GET, POST
        }

        public class DeviceVector
        {
            public int StartAddress = 0;
            public int Count = 1;
            public string DeviceAddress = "";
            public CommunicationMode mode = CommunicationMode.NONE;
            public SerialPort port;
            public string File = "";
            public string Data = "";
            public Dictionary<string,object> args = new Dictionary<string, object>();

            public void StartConnection()
            {
                if (mode == CommunicationMode.NONE)
                {
                    Console.WriteLine("ERROR: You need to provide communication mode");
                    Program.Exit(1);
                }
                if (mode == CommunicationMode.Serial)
                {
                    port = new SerialPort(DeviceAddress);
                    port.BaudRate = 115200;
                    port.Open();
                }
            }

            public void AssertFile()
            {
                if (File.Length <= 0)
                {
                    Console.WriteLine("ERROR: You have to specify file path");
                    Program.Exit(1);
                }

            }

            public ReadVector ToReadVector()
            {
                var rv = new ReadVector();
                rv.address = StartAddress;
                rv.count = Count;
                return rv;
            }

            public WriteVector ToWriteVector()
            {
                var rv = new WriteVector();
                rv.address = StartAddress;
                if (Data.Length > 0) rv.data = Data;
                else
                rv.data = System.IO.File.ReadAllText(File);
                return rv;
            }

            public WifiVector ToWifiVector()
            {
                var rv = new WifiVector();
                if (args.ContainsKey("ssid"))
                {
                    rv.ssid = (string)args["ssid"];
                }
                if (args.ContainsKey("pass"))
                {
                    rv.pass = (string)args["pass"];
                }
                if (args.ContainsKey("ap_ssid"))
                {
                    rv.ap_ssid = (string)args["ap_ssid"];
                }
                if (args.ContainsKey("ap_pass"))
                {
                    rv.ap_pass = (string)args["ap_pass"];
                }

                return rv;
            }

            public void Write(string data)
            {
                if (File.Length > 0)
                {
                    System.IO.File.WriteAllText(File, data);
                }
                else
                {
                    Console.WriteLine(data);
                }
            }

            public void Write(byte[] data)
            {
                if (File.Length > 0)
                {
                    System.IO.File.WriteAllBytes(File, data);
                }
                else
                {
                    Console.WriteLine(Encoding.ASCII.GetString(data));
                }
            }

            public void Write(Stream s)
            {
                StreamReader reader = new StreamReader(s);
                Write(reader.ReadToEnd());
            }

            public void AssertFileOrData()
            {
                if (File.Length <= 0 && Data.Length<=0)
                {
                    Console.WriteLine("ERROR: You have to specify file path or data");
                    Program.Exit(1);
                }
            }

            public void PrintRestSerial(bool toDone=false)
            {
                Console.WriteLine(ReadRestSerial(toDone));
            }

            public string ReadRestSerial(bool toDone=false)
            {
                if (toDone)
                {
                    return (port.ReadTo("done")) + port.ReadExisting();
                }
                return (port.ReadTo("\0")) + port.ReadExisting();
            }
        }

        public class ReadVector
        {
            public int address = 0;
            public int count = 1;
        }

        public class WifiVector
        {
            public string ssid = null;
            public string pass = null;
            public string ap_ssid = null;
            public string ap_pass = null;
        }

        public class WriteVector
        {
            public int address = 0;
            public string data = "";
        }

        public static string SendData(DeviceVector vector, REQUEST_TYPE type, string endpoint, string data = "")
        {
            if (vector.mode == CommunicationMode.Serial)
            {
                vector.port.ReadExisting();
                vector.port.Write($"c{type}\n/{endpoint}/\n{data}\n");
                return null;
            }
            if (vector.mode == CommunicationMode.HTTP)
            {
                WebClient wc = new WebClient();
                if(type == REQUEST_TYPE.POST)
                {
                    return wc.UploadString(new Uri(new Uri(vector.DeviceAddress), endpoint + "/"), data);
                }
                else
                {
                    return wc.DownloadString(new Uri(new Uri(vector.DeviceAddress), endpoint + "/"));

                }
            }
            return null;
        }

        public static void Upload(DeviceVector vector)
        {
            vector.AssertFileOrData();
            vector.StartConnection();
            var json = JsonConvert.SerializeObject(vector.ToWriteVector(), Formatting.None);
            var data = SendData(vector, REQUEST_TYPE.POST, "write", json);
            Program.Exit(0);
        }

        public static void Read(DeviceVector vector)
        {
            vector.StartConnection();
            var json = JsonConvert.SerializeObject(vector.ToReadVector(), Formatting.None);
            var data = SendData(vector, REQUEST_TYPE.POST, "read", json);

            string output = "";

            if (vector.mode == CommunicationMode.Serial)
            {
                output = vector.ReadRestSerial();
            }
            if (vector.mode == CommunicationMode.HTTP)
            {
                output = data;
            }

            Regex r = new Regex(",$");
            output = r.Replace(output, "");
            string aout = "";
            var blocks = output.Split(',');
            foreach (var item in blocks)
            {
                aout += Encoding.ASCII.GetString(Convert.FromBase64String(item));
            }
            vector.Write(aout);


            Program.Exit(0);
        }

        public static void Reset(DeviceVector vector)
        {
            if (vector.mode != CommunicationMode.Serial)
            {
                Console.WriteLine("ERROR: Reset can only be performed on Serial");
                Program.Exit(1);
            }
            vector.StartConnection();

            vector.port.Write($"ry");
            //vector.PrintRestSerial(true);
            Program.Exit(0);
        }

        public static void PinConfGet(DeviceVector vector)
        {
            vector.StartConnection();
            var data = SendData(vector, REQUEST_TYPE.GET, "pinconf");

            byte[] b = new byte[vector.Count];

            if (vector.mode == CommunicationMode.Serial)
            {
                vector.port.Read(b, 0, vector.Count);
                vector.Write(b);
            }
            if (vector.mode == CommunicationMode.HTTP)
            {
                vector.Write(data);
            }
            Program.Exit(0);
        }

        public static void PinConfSet(DeviceVector vector)
        {
            vector.StartConnection();
            var data = SendData(vector, REQUEST_TYPE.POST, "pinconf", vector.ToWriteVector().data);
            Program.Exit(0);
        }

        public static void WifiConfSet(DeviceVector vector)
        {
            vector.StartConnection();
            var json = JsonConvert.SerializeObject(vector.ToWifiVector(), Formatting.None);
            var data = SendData(vector, REQUEST_TYPE.POST, "wificonf", json);
            vector.PrintRestSerial(true);
            Program.Exit(0);
        }

        public static void WifiConfGet(DeviceVector vector)
        {
            if(vector.mode!= CommunicationMode.Serial)
            {
                Console.WriteLine("ERROR: Wificonf get only on serial");
                Program.Exit(1);
            }
            vector.StartConnection();
            vector.port.Write($"w");
            vector.PrintRestSerial();
            Program.Exit(0);
        }

        public static void DebugMode(DeviceVector vector, bool mode)
        {
            if (vector.mode != CommunicationMode.Serial)
            {
                Console.WriteLine("ERROR: DebugMode get only on serial");
                Program.Exit(1);
            }
            vector.StartConnection();
            vector.port.Write($"d"+(mode?'\u0001':'\u0000'));
            vector.PrintRestSerial();
            Program.Exit(0);
        }

        public static void IP_Data(DeviceVector vector)
        {
            if (vector.mode != CommunicationMode.Serial)
            {
                Console.WriteLine("ERROR: IpData get only on serial");
                Program.Exit(1);
            }
            vector.StartConnection();
            vector.port.Write($"ip");
            vector.PrintRestSerial();
            Program.Exit(0);
        }
    }
}
