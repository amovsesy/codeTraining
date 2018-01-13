using System;
using System.Data;
using System.Data.SqlClient;
using System.Windows.Forms;

public class MainForm : Form
{
    private MainMenu mmMenu;
    private MenuItem file;
    private MenuItem fileExit;
    private MenuItem fileSep;
    private MenuItem fileNewAppointment;
    private MenuItem fileSave;
    private MenuItem fileDelete;

    private ComboBox dropDownCities;

    private DataGrid carMakersTable;

    private const string CONSTR = @"Integrated Security=SSPI;database=CARS;server=IRINA\SQLEXPRESS";

    public MainForm()
    {
        Height = 700;
        Width = 1000;

        mmMenu = new MainMenu();
        file = new MenuItem("&File");
        fileExit = new MenuItem("E&xit");
        fileSep = new MenuItem("-");
        fileSave = new MenuItem("&Save");
        fileNewAppointment = new MenuItem("&New Appointment");
        fileDelete = new MenuItem("&Delete");

        file.MenuItems.Add(fileNewAppointment);
        file.MenuItems.Add(fileSave);
        file.MenuItems.Add(fileDelete);
        file.MenuItems.Add(fileSep);
        file.MenuItems.Add(fileExit);

        mmMenu.MenuItems.Add(file);

        dropDownCities = new ComboBox();
        dropDownCities.Location = new System.Drawing.Point(25, 25);
        dropDownCities.Width = 150;

        carMakersTable = new DataGrid();
        carMakersTable.Location = new System.Drawing.Point(0, 0);
        carMakersTable.Width = 950;
        carMakersTable.Height = 650;

        /*
         * SqlConnection con = new SqlConnection(CONSTR);

        SqlCommand cmd = new SqlCommand();
        cmd.Connection = con;
        cmd.CommandType = CommandType.Text;
        cmd.CommandText = @"select distinct maker from car_makers";

        con.Open();
        SqlDataReader rdr = cmd.ExecuteReader();

        while (rdr.Read())
        {
            dropDownCities.Items.Add(rdr.GetString(0));
        }

        rdr.Close();
        con.Close();
         */

        SqlConnection con = new SqlConnection(CONSTR);
        SqlCommand cmd = new SqlCommand(@"select * from cars_data", con);
        SqlDataAdapter da = new SqlDataAdapter();
        
        da.SelectCommand = cmd;
        con.Open();

        DataSet ds = new DataSet();

        da.Fill(ds);
        con.Close();

        dropDownCities.DataSource = ds.Tables[0];
        dropDownCities.DisplayMember = "maker";

        carMakersTable.DataSource = ds.Tables[0];

        dropDownCities.SelectedIndexChanged += new EventHandler(dropDownCities_SelectedIndexChanged);

        Menu = mmMenu;

        fileExit.Click += new EventHandler(fileExit_Click);
        fileSave.Click += new EventHandler(fileSave_Click);
        fileDelete.Click += new EventHandler(fileDelete_Click);
        fileNewAppointment.Click += new EventHandler(fileNewAppointment_Click);

        //Controls.Add(dropDownCities);
        Controls.Add(carMakersTable);
        Text = "Scheduler";
        StartPosition = FormStartPosition.CenterScreen;
        Show();
    }

    private void fileExit_Click(object sender, EventArgs e)
    {
        Close();
    }

    private void fileSave_Click(object sender, EventArgs e)
    {
        MessageBox.Show("Nothing to save.");
    }

    private void fileDelete_Click(object sender, EventArgs e)
    {
        MessageBox.Show("You can not delete since nothing saved.");
    }

    private void fileNewAppointment_Click(object sender, EventArgs e)
    {
        MessageBox.Show("No soup for you sorry.");
    }

    private void dropDownCities_SelectedIndexChanged(object sender, EventArgs e)
    {
        MessageBox.Show("Selected Item: " + dropDownCities.Text + 
            "\nSelected Index: " + dropDownCities.SelectedIndex.ToString());
    }

    public static void Main()
    {
        Application.Run(new MainForm());
    }

    private void InitializeComponent()
    {
        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
        this.SuspendLayout();
        // 
        // MainForm
        // 
        this.AccessibleDescription = null;
        this.AccessibleName = null;
        resources.ApplyResources(this, "$this");
        this.BackgroundImage = null;
        this.Font = null;
        this.Icon = null;
        this.Name = "MainForm";
        this.ResumeLayout(false);

    }
}