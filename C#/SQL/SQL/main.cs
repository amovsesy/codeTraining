using System;
using System.Data;
using System.Data.SqlClient;

public class SQLPractice
{
    public static void Main()
    {
        SqlConnection con = new SqlConnection("Persist Security Info=False;Integrated Security=SSPI;database=CARS;server=IRINA\\SQLEXPRESS;Connect Timeout=30");
        con.Open();

        Console.WriteLine("Connection String: " + con.ConnectionString);
        Console.WriteLine("DB: " + con.Database);
        Console.WriteLine("Data Source: " + con.DataSource);

        con.Close();
    }
}