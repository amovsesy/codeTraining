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

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            string name = textBox1.Text;

            if (name.Length != 0)
            {
                label1.Text = name + ", you can now change your name!";
                MessageBox.Show("Hello " + name + "\nHow do you like the picture?");
            }
            else
            {
                MessageBox.Show("Please Enter Your Name into the Field Provided!");
            }
        }
    }
}
