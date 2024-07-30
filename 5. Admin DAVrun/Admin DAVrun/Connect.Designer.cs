namespace Admin_DAVrun
{
    partial class Connect
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Connect));
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.textBoxPasswd = new System.Windows.Forms.TextBox();
            this.textBoxIP = new System.Windows.Forms.TextBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.btnConnect = new System.Windows.Forms.Button();
            this.textBoxLogin = new System.Windows.Forms.TextBox();
            this.TextIP = new System.Windows.Forms.Label();
            this.TextPort = new System.Windows.Forms.Label();
            this.Info1 = new System.Windows.Forms.Label();
            this.TextLogin = new System.Windows.Forms.Label();
            this.TextPasswd = new System.Windows.Forms.Label();
            this.TextStatusConnect = new System.Windows.Forms.Label();
            this.StatusConnect = new System.Windows.Forms.Panel();
            this.Info2 = new System.Windows.Forms.Label();
            this.checkBoxSave = new System.Windows.Forms.CheckBox();
            this.btnSettings = new System.Windows.Forms.Button();
            this.btnOpenClocePasswd = new System.Windows.Forms.Button();
            this.StatusConnect.SuspendLayout();
            this.SuspendLayout();
            // 
            // textBoxPasswd
            // 
            resources.ApplyResources(this.textBoxPasswd, "textBoxPasswd");
            this.textBoxPasswd.Name = "textBoxPasswd";
            // 
            // textBoxIP
            // 
            resources.ApplyResources(this.textBoxIP, "textBoxIP");
            this.textBoxIP.Name = "textBoxIP";
            this.textBoxIP.TextChanged += new System.EventHandler(this.textBoxIP_TextChanged);
            // 
            // textBoxPort
            // 
            resources.ApplyResources(this.textBoxPort, "textBoxPort");
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.TextChanged += new System.EventHandler(this.textBoxPort_TextChanged);
            this.textBoxPort.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textBoxPort_KeyPress);
            // 
            // btnConnect
            // 
            resources.ApplyResources(this.btnConnect, "btnConnect");
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click_1);
            // 
            // textBoxLogin
            // 
            resources.ApplyResources(this.textBoxLogin, "textBoxLogin");
            this.textBoxLogin.Name = "textBoxLogin";
            // 
            // TextIP
            // 
            resources.ApplyResources(this.TextIP, "TextIP");
            this.TextIP.Name = "TextIP";
            // 
            // TextPort
            // 
            resources.ApplyResources(this.TextPort, "TextPort");
            this.TextPort.Name = "TextPort";
            // 
            // Info1
            // 
            resources.ApplyResources(this.Info1, "Info1");
            this.Info1.Name = "Info1";
            // 
            // TextLogin
            // 
            resources.ApplyResources(this.TextLogin, "TextLogin");
            this.TextLogin.Name = "TextLogin";
            // 
            // TextPasswd
            // 
            resources.ApplyResources(this.TextPasswd, "TextPasswd");
            this.TextPasswd.Name = "TextPasswd";
            // 
            // TextStatusConnect
            // 
            resources.ApplyResources(this.TextStatusConnect, "TextStatusConnect");
            this.TextStatusConnect.Name = "TextStatusConnect";
            // 
            // StatusConnect
            // 
            this.StatusConnect.BackColor = System.Drawing.SystemColors.ControlLight;
            this.StatusConnect.Controls.Add(this.TextStatusConnect);
            resources.ApplyResources(this.StatusConnect, "StatusConnect");
            this.StatusConnect.Name = "StatusConnect";
            // 
            // Info2
            // 
            resources.ApplyResources(this.Info2, "Info2");
            this.Info2.ForeColor = System.Drawing.Color.Blue;
            this.Info2.Name = "Info2";
            // 
            // checkBoxSave
            // 
            resources.ApplyResources(this.checkBoxSave, "checkBoxSave");
            this.checkBoxSave.Name = "checkBoxSave";
            this.checkBoxSave.UseVisualStyleBackColor = true;
            this.checkBoxSave.CheckedChanged += new System.EventHandler(this.checkBoxSave_CheckedChanged);
            // 
            // btnSettings
            // 
            this.btnSettings.Image = global::Admin_DAVrun.Properties.Resources.Settings;
            resources.ApplyResources(this.btnSettings, "btnSettings");
            this.btnSettings.Name = "btnSettings";
            this.btnSettings.UseVisualStyleBackColor = true;
            this.btnSettings.Click += new System.EventHandler(this.btnSettings_Click);
            // 
            // btnOpenClocePasswd
            // 
            this.btnOpenClocePasswd.Image = global::Admin_DAVrun.Properties.Resources.ClosePasswd;
            resources.ApplyResources(this.btnOpenClocePasswd, "btnOpenClocePasswd");
            this.btnOpenClocePasswd.Name = "btnOpenClocePasswd";
            this.btnOpenClocePasswd.UseVisualStyleBackColor = true;
            this.btnOpenClocePasswd.Click += new System.EventHandler(this.btnOpenClocePasswd_Click);
            // 
            // Connect
            // 
            this.AcceptButton = this.btnConnect;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.Tan;
            this.Controls.Add(this.btnSettings);
            this.Controls.Add(this.checkBoxSave);
            this.Controls.Add(this.btnOpenClocePasswd);
            this.Controls.Add(this.Info2);
            this.Controls.Add(this.textBoxPasswd);
            this.Controls.Add(this.textBoxIP);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.textBoxLogin);
            this.Controls.Add(this.TextIP);
            this.Controls.Add(this.TextPort);
            this.Controls.Add(this.Info1);
            this.Controls.Add(this.TextLogin);
            this.Controls.Add(this.TextPasswd);
            this.Controls.Add(this.StatusConnect);
            this.MaximizeBox = false;
            this.Name = "Connect";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.StatusConnect.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
        private System.Windows.Forms.TextBox textBoxPasswd;
        private System.Windows.Forms.TextBox textBoxIP;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.TextBox textBoxLogin;
        private System.Windows.Forms.Label TextIP;
        private System.Windows.Forms.Label TextPort;
        private System.Windows.Forms.Label Info1;
        private System.Windows.Forms.Label TextLogin;
        private System.Windows.Forms.Label TextPasswd;
        private System.Windows.Forms.Label TextStatusConnect;
        private System.Windows.Forms.Panel StatusConnect;
        private System.Windows.Forms.Label Info2;
        private System.Windows.Forms.Button btnOpenClocePasswd;
        private System.Windows.Forms.CheckBox checkBoxSave;
        private System.Windows.Forms.Button btnSettings;
    }
}