<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class frmMain
    Inherits System.Windows.Forms.Form

    'Form esegue l'override del metodo Dispose per pulire l'elenco dei componenti.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Richiesto da Progettazione Windows Form
    Private components As System.ComponentModel.IContainer

    'NOTA: la procedura che segue è richiesta da Progettazione Windows Form
    'Può essere modificata in Progettazione Windows Form.  
    'Non modificarla mediante l'editor del codice.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(frmMain))
        Me.SerialCOM = New System.IO.Ports.SerialPort(Me.components)
        Me.txtMonitor = New System.Windows.Forms.TextBox()
        Me.btnMonitorClear = New System.Windows.Forms.Button()
        Me.SuspendLayout()
        '
        'SerialCOM
        '
        Me.SerialCOM.BaudRate = 115200
        Me.SerialCOM.PortName = "COM10"
        '
        'txtMonitor
        '
        Me.txtMonitor.Font = New System.Drawing.Font("Courier New", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.txtMonitor.Location = New System.Drawing.Point(22, 18)
        Me.txtMonitor.Multiline = True
        Me.txtMonitor.Name = "txtMonitor"
        Me.txtMonitor.ScrollBars = System.Windows.Forms.ScrollBars.Vertical
        Me.txtMonitor.Size = New System.Drawing.Size(586, 378)
        Me.txtMonitor.TabIndex = 0
        '
        'btnMonitorClear
        '
        Me.btnMonitorClear.Location = New System.Drawing.Point(624, 357)
        Me.btnMonitorClear.Name = "btnMonitorClear"
        Me.btnMonitorClear.Size = New System.Drawing.Size(164, 39)
        Me.btnMonitorClear.TabIndex = 1
        Me.btnMonitorClear.Text = "Clear"
        Me.btnMonitorClear.UseVisualStyleBackColor = True
        '
        'frmMain
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(800, 450)
        Me.Controls.Add(Me.btnMonitorClear)
        Me.Controls.Add(Me.txtMonitor)
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.Name = "frmMain"
        Me.Text = "Serial Monitor"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub

    Friend WithEvents SerialCOM As IO.Ports.SerialPort
    Friend WithEvents txtMonitor As TextBox
    Friend WithEvents btnMonitorClear As Button
End Class
