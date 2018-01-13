namespace GUI
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(300, 300);
            this.Name = "My GUI";
            this.Text = "GUI Example";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);


            this.helloLabel = new System.Windows.Forms.Label();
            this.helloLabel.AutoSize = true;
            this.helloLabel.Location = new System.Drawing.Point(0, 0);
            this.helloLabel.Name = "Hello Label";
            this.helloLabel.Text = "Hi Mama!!";

            this.helloButton = new System.Windows.Forms.Button();
            this.helloButton.AutoSize = true;
            this.helloButton.Location = new System.Drawing.Point(60, 270);
            this.helloButton.Name = "Mama's Button";
            this.helloButton.Text = "Mama's Button, so Mama Click Here";

            this.helloButton.Click += new System.EventHandler(this.buttonClicked);

            this.Controls.Add(this.helloLabel);
            this.Controls.Add(this.helloButton);
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;

        }

        #endregion
    }
}