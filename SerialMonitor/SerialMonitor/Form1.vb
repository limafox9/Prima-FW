Imports System.IO.Ports

Public Class frmMain
    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        SerialCOM.Open()

    End Sub

    Private Sub RxComData(s As String)
        ''txtMonitor.Text += s
        txtMonitor.AppendText(s)
        txtMonitor.Select(txtMonitor.TextLength, 0)
        txtMonitor.ScrollToCaret()

    End Sub

    Private Sub SerialPort1_DataReceived(sender As Object, e As SerialDataReceivedEventArgs) Handles SerialCOM.DataReceived
        Me.Invoke(Sub()
                      RxComData(SerialCOM.ReadExisting)
                  End Sub)
    End Sub

    Private Sub btnMonitorClear_Click(sender As Object, e As EventArgs) Handles btnMonitorClear.Click
        txtMonitor.Text = ""
    End Sub

End Class
