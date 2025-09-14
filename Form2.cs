using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;

namespace SLMC60X_Tool
{
    public partial class Form2 : Form
    {
        public static Form2 form2;
        /******************变量定义***************************/
        #region 变量定义
        //public const byte START_SYMBOL = 0xA8;
        //public const byte RS485_SlavePC_ID = 0xFC;
        //public const byte RS485_Slave01_ID = 0xA1;
        //public const byte RS485_Slave02_ID = 0xA2;
        //public const byte RS485_Slave03_ID = 0xA3;
        //public const byte RS485_FUN_COMM1 = 0x01;
        //public const byte RS485_FUN_COMM2 = 0x02;
        //public const byte RS485_FUN_COMM3 = 0x03;
        //public const byte RS485_FUN_COMM4 = 0x04;
        //public const byte SER_QUEUE_MAX = 32;

        public byte[,] ReceData = new byte[32, 255];
        public byte writepos = 0;
        public byte readpos = 0;
        public int RecCount = 0;
        public int SenCount = 0; 
        public int ReceErr = 0;
        #endregion

        public Form2()
        {
            InitializeComponent();
            string[] comPorts = SerialPort.GetPortNames();   //获取可以使用的串口
            foreach (string port in comPorts)
            {
                comboBox1.Items.Add(port);
            }
            form2 = this;
        }

        /******************打开串口***************************/
        #region 打开串口
        private void button1_Click(object sender, EventArgs e)
        {
            if (button1.Text == "打开串口") //打开串口
            {
                try
                {
                    serialPort1.PortName = comboBox1.Text;
                    serialPort1.BaudRate = Convert.ToInt32(comboBox2.Text, 10);
                    serialPort1.DataBits = Convert.ToInt32(comboBox3.Text, 10);
                    switch (comboBox4.Text)
                    {
                        case "无":
                            serialPort1.Parity = Parity.None;
                            break;
                        case "奇校验":
                            serialPort1.Parity = Parity.Odd;
                            break;
                        case "偶校验":
                            serialPort1.Parity = Parity.Even;
                            break;
                        default:
                            serialPort1.Parity = Parity.None;
                            break;

                    }
                    switch (comboBox5.Text)
                    {
                        case "0":
                            serialPort1.StopBits = StopBits.None;
                            break;
                        case "1":
                            serialPort1.StopBits = StopBits.One;
                            break;
                        case "2":
                            serialPort1.StopBits = StopBits.Two;
                            break;
                        case "1.5":
                            serialPort1.StopBits = StopBits.OnePointFive;
                            break;
                        default:
                            serialPort1.StopBits = StopBits.One;
                            break;

                    }
                    serialPort1.Open();
                    (this.Owner as Form1).toolStripStatusLabel1.BackColor = Color.Green;
                    (this.Owner as Form1).toolStripStatusLabel1.Text = "运行";
                    button1.Text = "关闭串口";
                    pictureBox1.BackColor = Color.Green;
                    (this.Owner as Form1).timer1.Enabled = true;
                    (this.Owner as Form1).timer2.Enabled = true;
                    serialPort1.ReceivedBytesThreshold = 1;
                    serialPort1.DataReceived += new SerialDataReceivedEventHandler(SerialPort1_DataReceived);//必须手动添加事件处理程序 
                }
                catch
                {
                    button1.Text = "打开串口";
                    (this.Owner as Form1).timer1.Enabled = false;
                    pictureBox1.BackColor = Color.DimGray;

                    MessageBox.Show("端口错误,请检查串口", "错误");
                }
            }
            else //关闭串口
            {
                (this.Owner as Form1).toolStripStatusLabel1.BackColor = Color.LightGray;
                (this.Owner as Form1).toolStripStatusLabel1.Text = "停止";
                serialPort1.Close();
                if (!serialPort1.IsOpen)
                {
                    button1.Text = "打开串口";
                    pictureBox1.BackColor = Color.LightGray;
                }
            }
        }

        private void button2_Click(object sender, EventArgs e) //刷新串口端口
        {
            comboBox1.Items.Clear();
            string[] comPorts = SerialPort.GetPortNames();   //获取可以使用的串口
            foreach (string port in comPorts)
            {
                comboBox1.Items.Add(port);
            }
        }
        #endregion

        /******************串口接收函数***************************/
        #region 串口接收函数
        private void SerialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)//串口数据接收事件
        {
            int DataLengh = 0;
            int ReceSta = 0; //当前接受第几类数据
            int ReceCnt = 0; //当前接受第几个数据
            int ReceDataNumber = 0; //接受总个数
            int ReceCrc = 0;   //接收的数据累加校验
            byte[] ReceBuf = new byte[2000];

            System.Threading.Thread.Sleep(50);//延时10ms 等待接收完数据
            DataLengh = serialPort1.BytesToRead;
            serialPort1.Read(ReceBuf, 0, DataLengh);

            for (int i = 0; i < DataLengh; i++)
            {
                switch (ReceSta)
                {
                    case 0:
                        if (ReceBuf[i] == Form1.START_SYMBOL)  //起始接收数据
                        {
                            ReceData[writepos, 0] = ReceBuf[i];
                            ReceSta = 1;
                            ReceCnt = 1;
                            ReceCrc = ReceData[writepos, 0];
                        }
                        else
                        {
                            ReceSta = 0; //当前接受第几类数据
                            ReceCnt = 0; //当前接受第几个数据
                            ReceDataNumber = 0; //接受总个数
                            ReceCrc = 0;
                            ReceErr |= 0x01;//接受出错 
                        }
                        break;
                    case 1:
                            ReceData[writepos, ReceCnt] = ReceBuf[i];    //地址	
                            ReceCrc += ReceData[writepos, ReceCnt];
                            ReceSta++;
                            ReceCnt++;
                        break;
                    case 2:
                        if ((ReceBuf[i] > 0x00) && (ReceBuf[i] < 0x20))  //功能指令
                        {
                            ReceData[writepos, ReceCnt] = ReceBuf[i];
                            ReceCrc += ReceData[writepos, ReceCnt];
                            ReceSta ++;
                            ReceCnt ++;
                        }
                        else
                        {
                            ReceSta = 0; //当前接受第几类数据
                            ReceCnt = 0; //当前接受第几个数据
                            ReceDataNumber = 0; //接受总个数
                            ReceCrc = 0;
                            ReceErr |= 0x02;//接受出错 
                        }
                        break;
                    case 3:
                        ReceData[writepos, ReceCnt] = ReceBuf[i];   //计算数据个数
                        ReceCrc += ReceData[writepos, ReceCnt];
                        ReceDataNumber = ReceData[writepos, ReceCnt] + 5; //计算数据个数
                        ReceCnt++;
                        ReceSta++;
                        break;
                    case 4:
                        ReceData[writepos, ReceCnt] = ReceBuf[i];
                        ReceCrc += ReceData[writepos, ReceCnt];
                        ReceCnt++;
                        if (ReceData[writepos, 2] == 1)
                        {
                            ReceData[writepos, 2] = 1;
                        }

                        if (ReceCnt >= ReceDataNumber)  //数据个数相等
                        {
                            if ((ReceCrc & 0xff) == 0) //CRC校验
                            {
                                if (ReceData[writepos,2]==1)
                                {
                                    ReceData[writepos, 2] = 1;
                                }
                                if ((readpos - writepos) != 1)
                                {
                                    writepos++;
                                    if (writepos >= Form1.SER_QUEUE_MAX)
                                    {
                                        writepos = 0;
                                    }
                                }
                                ReceSta = 0; //当前接受第几类数据
                                ReceCnt = 0; //当前接受第几个数据
                                ReceDataNumber = 0; //接受总个数
                                ReceCrc = 0;
                                ReceErr = 0; //接受出错  
                            }
                            else
                            {
                                ReceSta = 0; //当前接受第几类数据
                                ReceCnt = 0; //当前接受第几个数据
                                ReceDataNumber = 0; //接受总个数
                                ReceCrc = 0;
                                ReceErr |= 0x04;//接受出错  crc校验出错
                            }
                        }
                        break;
                    default:
                        ReceSta = 0; //当前接受第几类数据
                        ReceCnt = 0; //当前接受第几个数据
                        ReceDataNumber = 0; //接受总个数
                        ReceCrc = 0;
                        ReceErr = 0x8; //接受出错
                        break;
                }
            }
            RecCount += DataLengh;
            (this.Owner as Form1).toolStripStatusLabel4.Text = Convert.ToString(RecCount);
        }
        #endregion

        /******************串口发送数据***************************/
        #region   串口发送数据
        public void SendFunction1(byte STData,byte FunCom,byte LengByte,byte[] Data)
        {
            byte i=0;
            byte CRCnum=0;
            byte[] SendData = new byte[255];

            SendData[0] = Form1.START_SYMBOL;
            CRCnum = Form1.START_SYMBOL;
            SendData[1]=STData;
            CRCnum+=STData;
            SendData[2]=FunCom;
            CRCnum+=FunCom;
            SendData[3]=LengByte;
            CRCnum+=LengByte;
            for(i=0;i<LengByte;i++)
            {    
                SendData[i+4]=Data[i];
                CRCnum+=Data[i];
            }
            SendData[LengByte+4]=(byte)(0x100-CRCnum);

            if (serialPort1.IsOpen)   //判断串口是否开
            {
                serialPort1.Write(SendData, 0, LengByte + 5);
                SenCount += LengByte + 5;
                (this.Owner as Form1).toolStripStatusLabel6.Text = Convert.ToString(SenCount);
            }
            else
            {
                MessageBox.Show("串口没有打开", "报警");//出错提示
            }
        }
        #endregion
    }
}
