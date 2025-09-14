using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SLMC60X_Tool
{
    public partial class Form1 : Form
    {

        Form2 Form2SPCOM = new Form2(); //创建一个新窗体
        Form3 Form3About = new Form3();//创建一个新窗体//
        //******************定义变量******************//
        #region   定义变量
        public const byte START_SYMBOL = 0xA8;
        public const byte RS485_BMS_ID = 0x11;
        public const byte RS485_FUN_COMM1 = 0x01;
        public const byte RS485_FUN_COMM2 = 0x02;
        public const byte RS485_FUN_COMM3 = 0x03;
        public const byte RS485_FUN_COMM4 = 0x04;
        public const byte SER_QUEUE_MAX = 32;
        public const byte COMM_DSG_ON = 1;
        public const byte COMM_DSG_OFF=2;
        public const byte  COMM_CHG_ON=3;
        public const byte  COMM_CHG_OFF=4;
        public const byte  COMM_CLEAR_ALERT=5;
        public const byte  COMM_ENTER_SHIP=6;

        public byte[] Sedata = new byte[200];
        public byte[] SerPortFree = new byte[10]; //待处理数据帧的编号
        //public byte ColumnDispNum = 0; // 
        public byte NumConut = 0;
        public byte CtrPCEN = 0;
        #endregion

        //******************定义结构体******************//
        #region   定义结构体
        unsafe public struct BatData
        { 
           public UInt32   BatVolt;
           public Int16   BatCurr;
           public UInt16 BQ769xx_Volt_Gian;
           public byte  BQ769xx_Volt_OffSet;
           public fixed UInt16 CellVolt[15];
           public fixed UInt16 Temp[3];
           public UInt16 SysSOC;
           public UInt32 Bala_ctrl_sw;

        }
        public BatData BatDataMeg;

        public struct BQ76xx_Reg
        { 
           public byte SysStatus;
           public byte SysCtrl1;
           public byte SysCtrl2;
           public byte Balance1;
           public byte Balance2;
           public byte Balance3;
           public byte Protect1;
           public byte Protect2;
           public byte Protect3;
           public byte OV_Trip;
           public byte UV_Trip;
           public byte CC_CFG;
        }
        public BQ76xx_Reg BQ76xx_RegG1;
        #endregion

        //******************系统初始化******************//
        #region   系统初始化
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            DataGridInitPara();
        }

        private void 串口设置ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form2SPCOM.Owner = this;  //声明子窗体
            Form2SPCOM.StartPosition = FormStartPosition.CenterParent;
            Form2SPCOM.ShowDialog();

        }
        private void 版本信息ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form3About.Owner = this;  //声明子窗体
            Form3About.StartPosition = FormStartPosition.CenterParent;
            Form3About.ShowDialog();
        }

        private void DataGridInitPara()
        {
            for (byte ia = 0; ia <15; ia++)
            {
                dataGridView1.Rows.Add();
                dataGridView1.Rows[ia].Height = 20;
                dataGridView1.Rows[ia].Cells[0].Value = Convert.ToString(ia + 1, 10);
            }
            for (Int16 ia = 0; ia < 4; ia++)// 禁止列自动排序
            {
                dataGridView1.Columns[ia].SortMode = DataGridViewColumnSortMode.NotSortable;
            }
            dataGridView1.ColumnHeadersHeight = 20;
            dataGridView1.RowHeadersWidth = 100;
            dataGridView1.AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.Fill;
            dataGridView1.AutoSizeRowsMode = DataGridViewAutoSizeRowsMode.AllCells;
        }
        #endregion

        //******************定时发送请求数据******************//
        #region   定时发送请求数据
        private void timer1_Tick(object sender, EventArgs e)
        {
            toolStripStatusLabel8.Text = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
            if (NumConut++ >= 5)
            {
                NumConut = 0;
            }
            switch (NumConut)
            {
                case 0:
                    if (Form2SPCOM.serialPort1.IsOpen) //
                    {
                        Sedata[0] = 0x00;
                        Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM1, 0x01, Sedata);
                    }
                    break;
                case 1:
                    if (Form2SPCOM.serialPort1.IsOpen) //
                    {
                        Sedata[0] = 0x01;
                        Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM2, 0x01, Sedata);
                    }
                    break;
                case 2:
                    if (Form2SPCOM.serialPort1.IsOpen) //
                    {
                        Sedata[0] = 0x00;
                        Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM3, 0x01, Sedata);
                    }
                    break;
                case 3:

                    break;
                case 4:

                    break;
                case 5:

                    break;
                default:
                    break;
            }
        }
        #endregion

        //******************显示接收的数据******************//
        #region   显示接收的数据
        private void timer2_Tick(object sender, EventArgs e)
        {
            if (Form2SPCOM.readpos != Form2SPCOM.writepos)
            {
                // Seria_Pro_Data(Form2SPCOM.ReceData[Form2SPCOM.readpos,0  ] 255);
                switch (Form2SPCOM.ReceData[Form2SPCOM.readpos, 1])
                {
                    case RS485_BMS_ID:
                        Seria_ProPC_Data();
                        break;
                    default: break;
                }
                Form2SPCOM.readpos++;
                if (Form2SPCOM.readpos >=SER_QUEUE_MAX)
                {
                    Form2SPCOM.readpos = 0;
                }
            }
        }

        private void Seria_ProPC_Data()
        {
            switch (Form2SPCOM.ReceData[Form2SPCOM.readpos, 2])
            {
                case RS485_FUN_COMM1:
                    ReceGianOffset();
                    break;
                case RS485_FUN_COMM2:
                    ReceBascData();
                    break;
                case RS485_FUN_COMM3:
                    ReceCongfigPara();
                    break;
                case RS485_FUN_COMM4:
                    break;
                default: break;
            }
    
        }

        private void ReceGianOffset()
        {
            BatDataMeg.BQ769xx_Volt_Gian = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 4] << 8) + (Form2SPCOM.ReceData[Form2SPCOM.readpos, 5]));
            BatDataMeg.BQ769xx_Volt_OffSet = Form2SPCOM.ReceData[Form2SPCOM.readpos, 6];
            textBox4.Text = Convert.ToString(BatDataMeg.BQ769xx_Volt_Gian);
            textBox3.Text = Convert.ToString(BatDataMeg.BQ769xx_Volt_OffSet);
        }

        unsafe private void ReceBascData()
        {
            Double receDataDou = 0;
            UInt16 rData = 0;
            UInt32 Balance_sw = 0;
            fixed (BatData* BatDataMegA = &BatDataMeg)
            {
                BatDataMegA->CellVolt[0] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 4] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 5]);
                BatDataMegA->CellVolt[1] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 6] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 7]);
                BatDataMegA->CellVolt[2] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 8] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 9]);
                BatDataMegA->CellVolt[3] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 10] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 11]);
                BatDataMegA->CellVolt[4] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 12] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 13]);
                BatDataMegA->CellVolt[5] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 14] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 15]);
                BatDataMegA->CellVolt[6] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 16] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 17]);
                BatDataMegA->CellVolt[7] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 18] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 19]);
                BatDataMegA->CellVolt[8] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 20] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 21]);
                BatDataMegA->CellVolt[9] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 22] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 23]);
                BatDataMegA->CellVolt[10] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 24] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 25]);
                BatDataMegA->CellVolt[11] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 26] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 27]);
                BatDataMegA->CellVolt[12] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 28] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 29]);
                BatDataMegA->CellVolt[13] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 30] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 31]);
                BatDataMegA->CellVolt[14] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 32] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 33]);

                BatDataMegA->BatVolt = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 34] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 35]);

                BatDataMegA->Temp[0] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 36] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 37]);
                BatDataMegA->Temp[1] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 38] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 39]);
                BatDataMegA->Temp[2] = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 40] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 41]);

                BatDataMegA->BatCurr = (short)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 42] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 43]);

                BatDataMegA->SysSOC = (UInt16)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 44] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 45]);

                BatDataMegA->Bala_ctrl_sw = (UInt32)((Form2SPCOM.ReceData[Form2SPCOM.readpos, 46] << 24) + (Form2SPCOM.ReceData[Form2SPCOM.readpos, 47] << 16) + (Form2SPCOM.ReceData[Form2SPCOM.readpos, 48] << 8) + Form2SPCOM.ReceData[Form2SPCOM.readpos, 49]);

                Balance_sw = BatDataMegA->Bala_ctrl_sw;
                for (byte ia = 0; ia < 15; ia++)
                {
                    receDataDou = (Double)BatDataMegA->CellVolt[ia] / 1000;
                    dataGridView1.Rows[ia].Cells[1].Value = Convert.ToString(receDataDou);
                    if ((Balance_sw & 0x01) == 0x01)
                    {
                        dataGridView1.Rows[ia].Cells[2].Value = "-->";
                    }
                    else
                    {
                        dataGridView1.Rows[ia].Cells[2].Value = " ";
                    }
                    Balance_sw = Balance_sw >> 1;
                }
                for (byte ia = 0; ia < 3; ia++)
                {
                    receDataDou = (Double)BatDataMegA->Temp[ia] / 10;
                    dataGridView1.Rows[ia].Cells[3].Value = Convert.ToString(receDataDou);
                }

                receDataDou = (Double)BatDataMegA->BatVolt / 1000;
                textBox1.Text = Convert.ToString(receDataDou);

                if (((BatDataMegA->BatCurr) & 0x8000) == 0x8000)
                {
                    rData = (UInt16)((BatDataMegA->BatCurr) & 0xFFFF);
                    rData = (UInt16)(0x10000 - (rData));
                    receDataDou = (Double)(rData) / 100;
                    textBox2.Text = "-" + Convert.ToString(receDataDou);
                }
                else
                {
                    receDataDou = (Double) (BatDataMegA->BatCurr) / 100;
                    textBox2.Text = Convert.ToString(receDataDou);
                }

                receDataDou = (Double)(BatDataMegA->SysSOC) / 10;
                textBox17.Text = Convert.ToString(receDataDou)+" %";
            }
        }

        private void ReceCongfigPara()
        {
            BQ76xx_RegG1.SysStatus = Form2SPCOM.ReceData[Form2SPCOM.readpos, 4];
            BQ76xx_RegG1.Balance1 = Form2SPCOM.ReceData[Form2SPCOM.readpos, 5];
            BQ76xx_RegG1.Balance2 = Form2SPCOM.ReceData[Form2SPCOM.readpos, 6];
            BQ76xx_RegG1.Balance3 = Form2SPCOM.ReceData[Form2SPCOM.readpos, 7];
            BQ76xx_RegG1.SysCtrl1 = Form2SPCOM.ReceData[Form2SPCOM.readpos, 8];
            BQ76xx_RegG1.SysCtrl2 = Form2SPCOM.ReceData[Form2SPCOM.readpos, 9];
            BQ76xx_RegG1.Protect1 = Form2SPCOM.ReceData[Form2SPCOM.readpos, 10];
            BQ76xx_RegG1.Protect2 = Form2SPCOM.ReceData[Form2SPCOM.readpos, 11];
            BQ76xx_RegG1.Protect3 = Form2SPCOM.ReceData[Form2SPCOM.readpos, 12];
            BQ76xx_RegG1.OV_Trip = Form2SPCOM.ReceData[Form2SPCOM.readpos, 13];
            BQ76xx_RegG1.UV_Trip = Form2SPCOM.ReceData[Form2SPCOM.readpos, 14];
            BQ76xx_RegG1.CC_CFG = Form2SPCOM.ReceData[Form2SPCOM.readpos, 15];

            BQ769xxRegShow();
        }

        private void BQ769xxRegShow()
        {
            if ((BQ76xx_RegG1.SysStatus & 0x01) == 0x01)
            {
                button1.BackColor = Color.SpringGreen;
            }
            else
            {
                button1.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.SysStatus & 0x02) == 0x02)
            {
                button2.BackColor = Color.SpringGreen;
            }
            else
            {
                button2.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.SysStatus & 0x04) == 0x04)
            {
                button3.BackColor = Color.SpringGreen;
            }
            else
            {
                button3.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.SysStatus & 0x08) == 0x08)
            {
                button4.BackColor = Color.SpringGreen;
            }
            else
            {
                button4.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.SysStatus & 0x10) == 0x10)
            {
                button5.BackColor = Color.SpringGreen;
            }
            else
            {
                button5.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.SysStatus & 0x20) == 0x20)
            {
                button6.BackColor = Color.SpringGreen;
            }
            else
            {
                button6.BackColor = Color.LightGray;

            }
            if ((BQ76xx_RegG1.SysStatus & 0x80) == 0x80)
            {
                button8.BackColor = Color.SpringGreen;
            }
            else
            {
                button8.BackColor = Color.LightGray;
            }

            textBox5.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysStatus, 16);

            if ((BQ76xx_RegG1.Balance1 & 0x01) == 0x01)
            {
                button99.BackColor = Color.SpringGreen;
            }
            else
            {
                button99.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Balance1 & 0x02) == 0x02)
            {
                button31.BackColor = Color.SpringGreen;
            }
            else
            {
                button31.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Balance1 & 0x04) == 0x04)
            {
                button30.BackColor = Color.SpringGreen;
            }
            else
            {
                button30.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Balance1 & 0x08) == 0x08)
            {
                button29.BackColor = Color.SpringGreen;
            }
            else
            {
                button29.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Balance1 & 0x10) == 0x10)
            {
                button28.BackColor = Color.SpringGreen;
            }
            else
            {
                button28.BackColor = Color.LightGray;
            }
            textBox8.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance1, 16);


            if ((BQ76xx_RegG1.Balance2 & 0x01) == 0x01)
            {
                button34.BackColor = Color.SpringGreen;
            }
            else
            {
                button34.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Balance2 & 0x02) == 0x02)
            {
                button33.BackColor = Color.SpringGreen;
            }
            else
            {
                button33.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Balance3 & 0x04) == 0x04)
            {
                button27.BackColor = Color.SpringGreen;
            }
            else
            {
                button27.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Balance2 & 0x08) == 0x08)
            {
                button26.BackColor = Color.SpringGreen;
            }
            else
            {
                button26.BackColor = Color.LightGray;
                
            }
            if ((BQ76xx_RegG1.Balance2 & 0x10) == 0x10)
            {
                button25.BackColor = Color.SpringGreen;
            }
            else
            {
                button25.BackColor = Color.LightGray;
            }
            textBox9.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance2, 16);


            if ((BQ76xx_RegG1.Balance3 & 0x01) == 0x01)
            {
                button39.BackColor = Color.SpringGreen;
            }
            else
            {
                button39.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Balance3 & 0x02) == 0x02)
            {
                button38.BackColor = Color.SpringGreen;
            }
            else
            {
                button38.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Balance3 & 0x04) == 0x04)
            {
                button37.BackColor = Color.SpringGreen;
            }
            else
            {
                button37.BackColor = Color.LightGray;

            }
            if ((BQ76xx_RegG1.Balance3 & 0x08) == 0x08)
            {
                button36.BackColor = Color.SpringGreen;
            }
            else
            {
                button36.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Balance3 & 0x10) == 0x10)
            {
                button35.BackColor = Color.SpringGreen;
            }
            else
            {
                button35.BackColor = Color.LightGray;
            }
            textBox10.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance3, 16);

            if ((BQ76xx_RegG1.SysCtrl1 & 0x01) == 0x01)
            {
                button16.BackColor = Color.SpringGreen;
            }
            else
            {
                button16.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.SysCtrl1 & 0x02) == 0x02)
            {
                button15.BackColor = Color.SpringGreen;
            }
            else
            {
                button15.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.SysCtrl1 & 0x08) == 0x08)
            {
                button13.BackColor = Color.SpringGreen;
            }
            else
            {
                button13.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.SysCtrl1 & 0x10) == 0x10)
            {
                button12.BackColor = Color.SpringGreen;
            }
            else
            {
                button12.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.SysCtrl1 & 0x80) == 0x80)
            {
                button9.BackColor = Color.SpringGreen;
            }
            else
            {
                button9.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.SysCtrl1 & 0x80) == 0x80)
            {
                button9.BackColor = Color.SpringGreen;
            }
            else
            {
                button9.BackColor = Color.LightGray;
            }
            textBox6.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl1, 16);

            if ((BQ76xx_RegG1.SysCtrl2 & 0x01) == 0x01)
            {
                button24.BackColor = Color.SpringGreen;
            }
            else
            {
                button24.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.SysCtrl2 & 0x02) == 0x02)
            {
                button23.BackColor = Color.SpringGreen;
            }
            else
            {
                button23.BackColor = Color.LightGray;

            }
            if ((BQ76xx_RegG1.SysCtrl2 & 0x20) == 0x20)
            {
                button19.BackColor = Color.SpringGreen;
            }
            else
            {
                button19.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect1 & 0x04) == 0x04)
            {
                button45.BackColor = Color.SpringGreen;
            }
            else
            {
                button45.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.SysCtrl2 & 0x80) == 0x80)
            {
                button17.BackColor = Color.SpringGreen;
            }
            else
            {
                button17.BackColor = Color.LightGray;
            }
            textBox7.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl2, 16);

            if ((BQ76xx_RegG1.Protect1 & 0x01) == 0x01)
            {
                button47.BackColor = Color.SpringGreen;
            }
            else
            {
                button47.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect1 & 0x02) == 0x02)
            {
                button46.BackColor = Color.SpringGreen;

            }
            else
            {
                button46.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect1 & 0x04) == 0x04)
            {
                button45.BackColor = Color.SpringGreen;
            }
            else
            {
                button45.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect1 & 0x08) == 0x08)
            {
                button44.BackColor = Color.SpringGreen;
            }
            else
            {
                button44.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect1 & 0x10) == 0x10)
            {
                button43.BackColor = Color.SpringGreen;
            }
            else
            {
                button43.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect1 & 0x80) == 0x80)
            {
                button48.BackColor = Color.SpringGreen;
            }
            else
            {
                button48.BackColor = Color.LightGray;
            }
            textBox11.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect1, 16);

            if ((BQ76xx_RegG1.Protect2 & 0x01) == 0x01)
            {
                button55.BackColor = Color.SpringGreen;
            }
            else
            {
                button55.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Protect2 & 0x02) == 0x02)
            {
                button54.BackColor = Color.SpringGreen;
            }
            else
            {
                button54.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Protect2 & 0x04) == 0x04)
            {
                button53.BackColor = Color.SpringGreen;
            }
            else
            {
                button53.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Protect2 & 0x08) == 0x08)
            {
                button52.BackColor = Color.SpringGreen;
            }
            else
            {
                button52.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect2 & 0x10) == 0x10)
            {
                button51.BackColor = Color.SpringGreen;
            }
            else
            {
                button51.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect2 & 0x20) == 0x20)
            {
                button50.BackColor = Color.SpringGreen;
            }
            else
            {
                button50.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Protect2 & 0x40) == 0x40)
            {
                button49.BackColor = Color.SpringGreen;
            }
            else
            {
                button49.BackColor = Color.LightGray;
            }
            textBox12.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect2, 16);

            if ((BQ76xx_RegG1.Protect3 & 0x10) == 0x10)
            {
                button59.BackColor = Color.SpringGreen;
            }
            else
            {
                button59.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Protect3 & 0x20) == 0x20)
            {
                button58.BackColor = Color.SpringGreen;
            }
            else
            {
                button58.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.Protect3 & 0x40) == 0x40)
            {
                button57.BackColor = Color.SpringGreen;
            }
            else
            {
                button57.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.Protect3 & 0x80) == 0x80)
            {
                button56.BackColor = Color.SpringGreen;
            }
            else
            {
                button56.BackColor = Color.LightGray;
            }
            textBox13.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect3, 16);

            if ((BQ76xx_RegG1.OV_Trip & 0x01) == 0x01)
            {
                button71.BackColor = Color.SpringGreen;
            }
            else
            {
                button71.BackColor = Color.LightGray;

            }
            if ((BQ76xx_RegG1.OV_Trip & 0x02) == 0x02)
            {
                button70.BackColor = Color.SpringGreen;
            }
            else
            {
                button70.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.OV_Trip & 0x04) == 0x04)
            {
                button69.BackColor = Color.SpringGreen;
            }
            else
            {
                button69.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.OV_Trip & 0x08) == 0x08)
            {
                button68.BackColor = Color.SpringGreen;
            }
            else
            {
                button68.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.OV_Trip & 0x10) == 0x10)
            {
                button67.BackColor = Color.SpringGreen;
            }
            else
            {
                button67.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.OV_Trip & 0x20) == 0x20)
            {
                button66.BackColor = Color.SpringGreen;
            }
            else
            {
                button66.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.OV_Trip & 0x40) == 0x40)
            {
                button65.BackColor = Color.SpringGreen;
            }
            else
            {
                button65.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.OV_Trip & 0x80) == 0x80)
            {
                button64.BackColor = Color.SpringGreen;
            }
            else
            {
                button64.BackColor = Color.LightGray;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);

            if ((BQ76xx_RegG1.UV_Trip & 0x01) == 0x01)
            {
                button79.BackColor = Color.SpringGreen;
            }
            else
            {
                button79.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.UV_Trip & 0x02) == 0x02)
            {
                button78.BackColor = Color.SpringGreen;
            }
            else
            {
                button78.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.UV_Trip & 0x04) == 0x04)
            {
                button77.BackColor = Color.SpringGreen;
            }
            else
            {
                button77.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.UV_Trip & 0x08) == 0x08)
            {
                button76.BackColor = Color.SpringGreen;
            }
            else
            {
                button76.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.UV_Trip & 0x10) == 0x10)
            {
                button75.BackColor = Color.SpringGreen;
            }
            else
            {
                button75.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.UV_Trip & 0x20) == 0x20)
            {
                button74.BackColor = Color.SpringGreen;
            }
            else
            {
                button74.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.UV_Trip & 0x40) == 0x40)
            {
                button81.BackColor = Color.SpringGreen;
            }
            else
            {
                button81.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.UV_Trip & 0x80) == 0x80)
            {
                button80.BackColor = Color.SpringGreen;
            }
            else
            {
                button80.BackColor = Color.LightGray;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);


            if ((BQ76xx_RegG1.CC_CFG & 0x01) == 0x01)
            {
                button87.BackColor = Color.SpringGreen;
            }
            else
            {
                button87.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.CC_CFG & 0x02) == 0x02)
            {
                button86.BackColor = Color.SpringGreen;
            }
            else
            {
                button86.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.UV_Trip & 0x04) == 0x04)
            {
                button77.BackColor = Color.SpringGreen;
            }
            else
            {
                button77.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.CC_CFG & 0x08) == 0x08)
            {
                button84.BackColor = Color.SpringGreen;
            }
            else
            {
                button84.BackColor = Color.LightGray;
            }

            if ((BQ76xx_RegG1.CC_CFG & 0x10) == 0x10)
            {
                button83.BackColor = Color.SpringGreen;
            }
            else
            {
                button83.BackColor = Color.LightGray;
            }
            if ((BQ76xx_RegG1.CC_CFG & 0x20) == 0x20)
            {
                button82.BackColor = Color.SpringGreen;
            }
            else
            {
                button82.BackColor = Color.LightGray;
            }
            textBox16.Text = "0x" + Convert.ToString(BQ76xx_RegG1.CC_CFG, 16);
        }
        #endregion

        //******************读 写 配置 设置等操作******************//
        #region   读 写 配置 设置等操作
        private void button89_Click(object sender, EventArgs e)  //读配置
        {
            if (Form2SPCOM.serialPort1.IsOpen) //
            {
                Sedata[0] = 0x01;
                Form2SPCOM.SendFunction1(0x08, 0x03, 0x01, Sedata);
            }
        }

        private void button88_Click(object sender, EventArgs e)  //写配置
        {
            if (Form2SPCOM.serialPort1.IsOpen)
            {
                Sedata[0] = Convert.ToByte(textBox5.Text, 16);
                Sedata[1] = Convert.ToByte(textBox5.Text, 16);
                Sedata[2] = Convert.ToByte(textBox5.Text, 16);
                Sedata[3] = Convert.ToByte(textBox5.Text, 16);
                Sedata[4] = Convert.ToByte(textBox5.Text, 16);
                Sedata[5] = Convert.ToByte(textBox5.Text, 16);
                Sedata[6] = Convert.ToByte(textBox5.Text, 16);
                Sedata[7] = Convert.ToByte(textBox5.Text, 16);
                Sedata[8] = Convert.ToByte(textBox5.Text, 16);
                Sedata[9] = Convert.ToByte(textBox5.Text, 16);
                Sedata[10] = Convert.ToByte(textBox5.Text, 16);
                Sedata[11] = Convert.ToByte(textBox5.Text, 16);

                Form2SPCOM.SendFunction1(0x08, 0x04, 0x0C, Sedata);
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysStatus & 0x01) == 0x01)
            {
                button1.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysStatus &= 0xFE;
            }
            else
            {
                button1.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysStatus |= 0x01;
            }
            textBox5.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysStatus, 16);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysStatus & 0x02) == 0x02)
            {
                button2.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysStatus &= 0xFD;
            }
            else
            {
                button2.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysStatus |= 0x02;
            }
            textBox5.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysStatus, 16);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysStatus & 0x04) == 0x04)
            {
                button3.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysStatus &= 0xFB;
            }
            else
            {
                button3.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysStatus |= 0x04;
            }
            textBox5.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysStatus, 16);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysStatus & 0x08) == 0x08)
            {
                button4.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysStatus &= 0xF7;
            }
            else
            {
                button4.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysStatus |= 0x08;
            }
            textBox5.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysStatus, 16);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysStatus & 0x10) == 0x10)
            {
                button5.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysStatus &= 0xEF;
            }
            else
            {
                button5.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysStatus |= 0x10;
            }
            textBox5.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysStatus, 16);
        }

        private void button6_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysStatus & 0x20) == 0x20)
            {
                button6.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysStatus &= 0xDF;
            }
            else
            {
                button6.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysStatus |= 0x20;
            }
            textBox5.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysStatus, 16);
        }

        private void button8_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysStatus & 0x80) == 0x80)
            {
                button8.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysStatus &= 0x7F;
            }
            else
            {
                button8.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysStatus |= 0x80;
            }
            textBox5.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysStatus, 16);
        }

        private void button99_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance1 & 0x01) == 0x01)
            {
                button99.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance1 &= 0xFE;
            }
            else
            {
                button99.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance1 |= 0x01;
            }
            textBox8.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance1, 16);

        }

        private void button31_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance1 & 0x02) == 0x02)
            {
                button31.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance1 &= 0xFD;
            }
            else
            {
                button31.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance1 |= 0x02;
            }
            textBox8.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance1, 16);
        }

        private void button30_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance1 & 0x04) == 0x04)
            {
                button30.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance1 &= 0xFB;
            }
            else
            {
                button30.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance1 |= 0x04;
            }
            textBox8.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance1, 16);
        }

        private void button29_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance1 & 0x08) == 0x08)
            {
                button29.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance1 &= 0xF7;
            }
            else
            {
                button29.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance1 |= 0x08;
            }
            textBox8.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance1, 16);
        }

        private void button28_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance1 & 0x10) == 0x10)
            {
                button28.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance1 &= 0xEF;
            }
            else
            {
                button28.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance1 |= 0x10;
            }
            textBox8.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance1, 16);
        }

        private void button34_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance2 & 0x01) == 0x01)
            {
                button34.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance2 &= 0xFE;
            }
            else
            {
                button34.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance2 |= 0x01;
            }
            textBox9.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance2, 16);
        }

        private void button33_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance2 & 0x02) == 0x02)
            {
                button33.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance2 &= 0xFD;
            }
            else
            {
                button33.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance2 |= 0x02;
            }
            textBox9.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance2, 16);
        }

        private void button27_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance2 & 0x04) == 0x04)
            {
                button27.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance2 &= 0xFB;
            }
            else
            {
                button27.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance2 |= 0x04;
            }
            textBox9.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance2, 16);

        }

        private void button26_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance2 & 0x08) == 0x08)
            {
                button26.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance2 &= 0xF7;
            }
            else
            {
                button26.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance2 |= 0x08;
            }
            textBox9.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance2, 16);
        }

        private void button25_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance2 & 0x10) == 0x10)
            {
                button25.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance2 &= 0xEF;
            }
            else
            {
                button25.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance2 |= 0x10;
            }
            textBox9.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance2, 16);
        }

        private void button39_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance3 & 0x01) == 0x01)
            {
                button39.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance3 &= 0xFE;
            }
            else
            {
                button39.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance3 |= 0x01;
            }
            textBox10.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance3, 16);
        }

        private void button38_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance3 & 0x02) == 0x02)
            {
                button38.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance3 &= 0xFD;
            }
            else
            {
                button38.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance3 |= 0x02;
            }
            textBox10.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance3, 16);
        }

        private void button37_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance3 & 0x04) == 0x04)
            {
                button37.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance3 &= 0xFB;
            }
            else
            {
                button37.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance3 |= 0x04;
            }
            textBox10.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance3, 16);
        }

        private void button36_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance3 & 0x08) == 0x08)
            {
                button36.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance3 &= 0xF7;
            }
            else
            {
                button36.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance3 |= 0x08;
            }
            textBox10.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance3, 16);
        }

        private void button35_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Balance3 & 0x10) == 0x10)
            {
                button35.BackColor = Color.LightGray;
                BQ76xx_RegG1.Balance3 &= 0xF7;
            }
            else
            {
                button35.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Balance3 |= 0x10;
            }
            textBox10.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Balance3, 16);
        }

        private void button16_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl1 & 0x01) == 0x01)
            {
                button16.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl1 &= 0xFE;
            }
            else
            {
                button16.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl1 |= 0x01;
            }
            textBox6.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl1, 16);
        }

        private void button15_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl1 & 0x02) == 0x02)
            {
                button15.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl1 &= 0xFD;
            }
            else
            {
                button15.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl1 |= 0x02;
            }
            textBox6.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl1, 16);
        }

        private void button13_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl1 & 0x08) == 0x08)
            {
                button13.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl1 &= 0xF7;
            }
            else
            {
                button13.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl1 |= 0x08;
            }
            textBox6.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl1, 16);
        }

        private void button12_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl1 & 0x10) == 0x10)
            {
                button12.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl1 &= 0xEF;
            }
            else
            {
                button12.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl1 |= 0x10;
            }
            textBox6.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl1, 16);
        }

        private void button9_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl1 & 0x80) == 0x80)
            {
                button9.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl1 &= 0x7F;
            }
            else
            {
                button9.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl1 |= 0x80;
            }
            textBox6.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl1, 16);
        }

        private void button24_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl2 & 0x01) == 0x01)
            {
                button24.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl2 &= 0xFE;
            }
            else
            {
                button24.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl2 |= 0x01;
            }
            textBox7.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl2, 16);
        }

        private void button23_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl2 & 0x02) == 0x02)
            {
                button23.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl2 &= 0xFD;
            }
            else
            {
                button23.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl2 |= 0x02;
            }
            textBox7.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl2, 16);
        }

        private void button19_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl2 & 0x20) == 0x20)
            {
                button19.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl2 &= 0xDF;
            }
            else
            {
                button19.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl2 |= 0x20;
            }
            textBox7.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl2, 16);
        }

        private void button18_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl2 & 0x40) == 0x40)
            {
                button18.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl2 &= 0xBF;
            }
            else
            {
                button18.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl2 |= 0x40;
            }
            textBox7.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl2, 16);
        }

        private void button17_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.SysCtrl2 & 0x80) == 0x80)
            {
                button17.BackColor = Color.LightGray;
                BQ76xx_RegG1.SysCtrl2 &= 0x7F;
            }
            else
            {
                button17.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.SysCtrl2 |= 0x80;
            }
            textBox7.Text = "0x" + Convert.ToString(BQ76xx_RegG1.SysCtrl2, 16);
        }

        private void button47_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect1 & 0x01) == 0x01)
            {
                button47.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect1 &= 0xFE;
            }
            else
            {
                button47.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect1 |= 0x01;
            }
            textBox11.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect1, 16);
        }

        private void button46_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect1 & 0x02) == 0x02)
            {
                button46.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect1 &= 0xFD;
            }
            else
            {
                button46.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect1 |= 0x02;
            }
            textBox11.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect1, 16);
        }

        private void button45_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect1 & 0x04) == 0x04)
            {
                button45.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect1 &= 0xFB;
            }
            else
            {
                button45.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect1 |= 0x04;
            }
            textBox11.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect1, 16);
        }

        private void button44_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect1 & 0x08) == 0x08)
            {
                button44.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect1 &= 0xF7;
            }
            else
            {
                button44.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect1 |= 0x08;
            }
            textBox11.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect1, 16);

        }

        private void button43_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect1 & 0x10) == 0x10)
            {
                button43.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect1 &= 0xEF;
            }
            else
            {
                button43.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect1 |= 0x10;
            }
            textBox11.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect1, 16);
        }

        private void button48_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect1 & 0x80) == 0x80)
            {
                button48.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect1 &= 0x7F;
            }
            else
            {
                button48.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect1 |= 0x80;
            }
            textBox11.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect1, 16);
        }

        private void button55_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect2 & 0x01) == 0x01)
            {
                button55.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect2 &= 0xFE;
            }
            else
            {
                button55.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect2 |= 0x01;
            }
            textBox12.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect2, 16);
        }

        private void button54_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect2 & 0x02) == 0x02)
            {
                button54.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect2 &= 0xFD;
            }
            else
            {
                button54.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect2 |= 0x02;
            }
            textBox12.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect2, 16);
        }

        private void button53_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect2 & 0x04) == 0x04)
            {
                button53.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect2 &= 0xFB;
            }
            else
            {
                button53.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect2 |= 0x04;
            }
            textBox12.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect2, 16);
        }

        private void button52_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect2 & 0x08) == 0x08)
            {
                button52.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect2 &= 0xF7;
            }
            else
            {
                button52.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect2 |= 0x08;
            }
            textBox12.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect2, 16);
        }

        private void button51_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect2 & 0x10) == 0x10)
            {
                button51.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect2 &= 0xEF;
            }
            else
            {
                button51.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect2 |= 0x10;
            }
            textBox12.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect2, 16);
        }

        private void button50_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect2 & 0x20) == 0x20)
            {
                button50.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect2 &= 0xDF;
            }
            else
            {
                button50.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect2 |= 0x20;
            }
            textBox12.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect2, 16);
        }

        private void button49_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect2 & 0x40) == 0x40)
            {
                button49.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect2 &= 0xBF;
            }
            else
            {
                button49.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect2 |= 0x40;
            }
            textBox12.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect2, 16);
        }

        private void button59_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect3 & 0x10) == 0x10)
            {
                button59.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect3 &= 0xEF;
            }
            else
            {
                button59.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect3 |= 0x10;
            }
            textBox13.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect3, 16);
        }

        private void button58_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect3 & 0x20) == 0x20)
            {
                button58.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect3 &= 0xDF;
            }
            else
            {
                button58.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect3 |= 0x20;
            }
            textBox13.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect3, 16);
        }

        private void button57_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect3 & 0x40) == 0x40)
            {
                button57.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect3 &= 0xBF;
            }
            else
            {
                button57.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect3 |= 0x40;
            }
            textBox13.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect3, 16);
        }

        private void button56_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.Protect3 & 0x80) == 0x80)
            {
                button56.BackColor = Color.LightGray;
                BQ76xx_RegG1.Protect3 &= 0x7F;
            }
            else
            {
                button56.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.Protect3 |= 0x80;
            }
            textBox13.Text = "0x" + Convert.ToString(BQ76xx_RegG1.Protect3, 16);
        }

        private void button71_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.OV_Trip & 0x01) == 0x01)
            {
                button71.BackColor = Color.LightGray;
                BQ76xx_RegG1.OV_Trip &= 0xFE;
            }
            else
            {
                button71.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.OV_Trip |= 0x01;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);
        }

        private void button70_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.OV_Trip & 0x02) == 0x02)
            {
                button70.BackColor = Color.LightGray;
                BQ76xx_RegG1.OV_Trip &= 0xFD;
            }
            else
            {
                button70.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.OV_Trip |= 0x02;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);
        }

        private void button69_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.OV_Trip & 0x04) == 0x04)
            {
                button69.BackColor = Color.LightGray;
                BQ76xx_RegG1.OV_Trip &= 0xFB;
            }
            else
            {
                button69.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.OV_Trip |= 0x04;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);
        }

        private void button68_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.OV_Trip & 0x08) == 0x08)
            {
                button68.BackColor = Color.LightGray;
                BQ76xx_RegG1.OV_Trip &= 0xF7;
            }
            else
            {
                button68.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.OV_Trip |= 0x08;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);
        }

        private void button67_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.OV_Trip & 0x10) == 0x10)
            {
                button67.BackColor = Color.LightGray;
                BQ76xx_RegG1.OV_Trip &= 0xEF;
            }
            else
            {
                button67.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.OV_Trip |= 0x10;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);
        }

        private void button66_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.OV_Trip & 0x20) == 0x20)
            {
                button66.BackColor = Color.LightGray;
                BQ76xx_RegG1.OV_Trip &= 0xDF;
            }
            else
            {
                button66.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.OV_Trip |= 0x20;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);
        }

        private void button65_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.OV_Trip & 0x40) == 0x40)
            {
                button65.BackColor = Color.LightGray;
                BQ76xx_RegG1.OV_Trip &= 0xBF;
            }
            else
            {
                button65.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.OV_Trip |= 0x40;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);
        }

        private void button64_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.OV_Trip & 0x80) == 0x80)
            {
                button64.BackColor = Color.LightGray;
                BQ76xx_RegG1.OV_Trip &= 0x7F;
            }
            else
            {
                button64.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.OV_Trip |= 0x80;
            }
            textBox14.Text = "0x" + Convert.ToString(BQ76xx_RegG1.OV_Trip, 16);
        }

        private void button79_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.UV_Trip & 0x01) == 0x01)
            {
                button79.BackColor = Color.LightGray;
                BQ76xx_RegG1.UV_Trip &= 0xFE;
            }
            else
            {
                button79.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.UV_Trip |= 0x01;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);
        }

        private void button78_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.UV_Trip & 0x02) == 0x02)
            {
                button78.BackColor = Color.LightGray;
                BQ76xx_RegG1.UV_Trip &= 0xFD;
            }
            else
            {
                button78.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.UV_Trip |= 0x02;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);
        }

        private void button77_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.UV_Trip & 0x04) == 0x04)
            {
                button77.BackColor = Color.LightGray;
                BQ76xx_RegG1.UV_Trip &= 0xFB;
            }
            else
            {
                button77.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.UV_Trip |= 0x04;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);
        }

        private void button76_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.UV_Trip & 0x08) == 0x08)
            {
                button76.BackColor = Color.LightGray;
                BQ76xx_RegG1.UV_Trip &= 0xF7;
            }
            else
            {
                button76.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.UV_Trip |= 0x08;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);
        }

        private void button75_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.UV_Trip & 0x10) == 0x10)
            {
                button75.BackColor = Color.LightGray;
                BQ76xx_RegG1.UV_Trip &= 0xEF;
            }
            else
            {
                button75.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.UV_Trip |= 0x10;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);
        }

        private void button74_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.UV_Trip & 0x20) == 0x20)
            {
                button74.BackColor = Color.LightGray;
                BQ76xx_RegG1.UV_Trip &= 0xDF;
            }
            else
            {
                button74.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.UV_Trip |= 0x20;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);
        }

        private void button81_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.UV_Trip & 0x40) == 0x40)
            {
                button81.BackColor = Color.LightGray;
                BQ76xx_RegG1.UV_Trip &= 0xBF;
            }
            else
            {
                button81.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.UV_Trip |= 0x40;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);
        }

        private void button80_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.UV_Trip & 0x80) == 0x80)
            {
                button80.BackColor = Color.LightGray;
                BQ76xx_RegG1.UV_Trip &= 0x7F;
            }
            else
            {
                button80.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.UV_Trip |= 0x80;
            }
            textBox15.Text = "0x" + Convert.ToString(BQ76xx_RegG1.UV_Trip, 16);
        }

        private void button87_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.CC_CFG & 0x01) == 0x01)
            {
                button87.BackColor = Color.LightGray;
                BQ76xx_RegG1.CC_CFG &= 0xFE;
            }
            else
            {
                button87.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.CC_CFG |= 0x01;
            }
            textBox16.Text = "0x" + Convert.ToString(BQ76xx_RegG1.CC_CFG, 16);
        }

        private void button86_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.CC_CFG & 0x02) == 0x02)
            {
                button86.BackColor = Color.LightGray;
                BQ76xx_RegG1.CC_CFG &= 0xFD;
            }
            else
            {
                button86.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.CC_CFG |= 0x02;
            }
            textBox16.Text = "0x" + Convert.ToString(BQ76xx_RegG1.CC_CFG, 16);
        }

        private void button85_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.CC_CFG & 0x04) == 0x04)
            {
                button85.BackColor = Color.LightGray;
                BQ76xx_RegG1.CC_CFG &= 0xFB;
            }
            else
            {
                button85.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.CC_CFG |= 0x04;
            }
            textBox16.Text = "0x" + Convert.ToString(BQ76xx_RegG1.CC_CFG, 16);
        }

        private void button84_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.CC_CFG & 0x08) == 0x08)
            {
                button84.BackColor = Color.LightGray;
                BQ76xx_RegG1.CC_CFG &= 0xF7;
            }
            else
            {
                button84.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.CC_CFG |= 0x08;
            }
            textBox16.Text = "0x" + Convert.ToString(BQ76xx_RegG1.CC_CFG, 16);
        }

        private void button83_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.CC_CFG & 0x10) == 0x10)
            {
                button83.BackColor = Color.LightGray;
                BQ76xx_RegG1.CC_CFG &= 0xEF;
            }
            else
            {
                button83.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.CC_CFG |= 0x10;
            }
            textBox16.Text = "0x" + Convert.ToString(BQ76xx_RegG1.CC_CFG, 16);
        }

        private void button82_Click(object sender, EventArgs e)
        {
            if ((BQ76xx_RegG1.CC_CFG & 0x20) == 0x20)
            {
                button82.BackColor = Color.LightGray;
                BQ76xx_RegG1.CC_CFG &= 0xDF;
            }
            else
            {
                button82.BackColor = Color.SpringGreen;
                BQ76xx_RegG1.CC_CFG |= 0x20;
            }
            textBox16.Text = "0x" + Convert.ToString(BQ76xx_RegG1.CC_CFG, 16);
        }
        #endregion
        //********************清除*********************************//
        private void button90_Click(object sender, EventArgs e)
        {  
            if (Form2SPCOM.serialPort1.IsOpen) //
            {  
                Sedata[0]=COMM_DSG_ON;
                Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM4, 0x01, Sedata);
            }
        }

        private void button91_Click(object sender, EventArgs e)
        {
            if (Form2SPCOM.serialPort1.IsOpen) //
            {
                Sedata[0] = COMM_DSG_OFF;
                Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM4, 0x01, Sedata);
            }
        }

        private void button93_Click(object sender, EventArgs e)
        {
            if (Form2SPCOM.serialPort1.IsOpen) //
            {
                Sedata[0] = COMM_CHG_ON;
                Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM4, 0x01, Sedata);
            }
        }

        private void button92_Click(object sender, EventArgs e)
        {
            if (Form2SPCOM.serialPort1.IsOpen) //
            {
                Sedata[0] = COMM_CHG_OFF;
                Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM4, 0x01, Sedata);
            }
        }
        private void button32_Click(object sender, EventArgs e)
        {
            if (Form2SPCOM.serialPort1.IsOpen) //
            {
                Sedata[0] = COMM_CLEAR_ALERT;
                Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM4, 0x01, Sedata);
            }
        }
        private void button94_Click(object sender, EventArgs e)
        {
            if (Form2SPCOM.serialPort1.IsOpen) //
            {
                Sedata[0] = COMM_ENTER_SHIP;
                Form2SPCOM.SendFunction1(RS485_BMS_ID, RS485_FUN_COMM4, 0x01, Sedata);
            }
        }

    }
}
