using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            
        }

        private void Form1_Click(object sender, EventArgs e)
        {
            int changeX = 1;
            int changeY = 1;
            int beforeX = this.pictureBox1.Location.X;
            int beforeY = this.pictureBox1.Location.Y;
            int afterX;
            int afterY;
            MessageBox.Show(this.Size.Height + " " + this.Size.Width);
            while (true)
            {
                afterX = beforeX + changeX;
                afterY = beforeY + changeY;

                if (afterX >= 900 || afterX < 0)
                {
                    changeX = 0 - changeX;
                    afterX = afterX + (2 * changeX);
                }

                if (afterY >= 458 || afterY < 0)
                {
                    changeY = 0 - changeY;
                    afterY = afterY + (2 * changeY);
                }

                this.pictureBox1.Location = new System.Drawing.Point(afterX, afterY);

                beforeX = this.pictureBox1.Location.X;
                beforeY = this.pictureBox1.Location.Y;
            }
        }
    }
}
