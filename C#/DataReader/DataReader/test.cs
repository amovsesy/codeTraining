using System;
using System.Data;
using System.Data.SqlClient;

public class DataReader
{
    private const string CONSTR = @"Integrated Security=SSPI;database=CARS;server=IRINA\SQLEXPRESS";

    public static void Main()
    {
        SqlConnection con = new SqlConnection(CONSTR);

        SqlCommand cmd = new SqlCommand();
        cmd.Connection = con;
        cmd.CommandType = CommandType.Text;
        cmd.CommandText = @"select distinct maker from car_makers order by maker";

        con.Open();
        SqlDataReader rdr = cmd.ExecuteReader();

        while (rdr.Read())
        {
            Console.WriteLine(rdr.GetString(0));
        }

        rdr.Close();
        con.Close();
    }
}