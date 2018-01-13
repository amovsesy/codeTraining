using System;
using System.Windows.Forms;

public class GUI
{
    public static void Main()
    {
        Form frmMain = new Form();
        TextBox txt1 = new TextBox();
        Label label = new Label();
        CheckBox check = new CheckBox();
        Button yes = new Button();
        

        frmMain.Text = "Simple GUI";
        frmMain.StartPosition = FormStartPosition.CenterScreen;

        txt1.Location = new System.Drawing.Point(20, 20);

        label.Text = "Hi Mama!";
        label.Location = new System.Drawing.Point(0, 0);

        check.Text = "Do you like?";
        check.Location = new System.Drawing.Point(40, 40);

        yes.Text = "Click here if yes.";
        yes.Location = new System.Drawing.Point(60, 60);
        yes.AutoSize = true;

        frmMain.Controls.Add(txt1);
        frmMain.Controls.Add(label);
        frmMain.Controls.Add(check);
        frmMain.Controls.Add(yes);
        frmMain.Show();

        Application.Run(frmMain);
    }
}