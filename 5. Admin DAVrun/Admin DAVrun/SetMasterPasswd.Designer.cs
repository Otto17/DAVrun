namespace Admin_DAVrun
{
    partial class SetMasterPasswd
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SetMasterPasswd));
            this.TextPasswd1 = new System.Windows.Forms.Label();
            this.TextPasswd2 = new System.Windows.Forms.Label();
            this.Info1 = new System.Windows.Forms.Label();
            this.SetPasswd1 = new System.Windows.Forms.TextBox();
            this.SetPasswd2 = new System.Windows.Forms.TextBox();
            this.btnSave = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // TextPasswd1
            // 
            this.TextPasswd1.AutoSize = true;
            this.TextPasswd1.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.TextPasswd1.Location = new System.Drawing.Point(12, 48);
            this.TextPasswd1.Name = "TextPasswd1";
            this.TextPasswd1.Size = new System.Drawing.Size(102, 15);
            this.TextPasswd1.TabIndex = 0;
            this.TextPasswd1.Text = "Введите пароль:";
            // 
            // TextPasswd2
            // 
            this.TextPasswd2.AutoSize = true;
            this.TextPasswd2.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.TextPasswd2.Location = new System.Drawing.Point(12, 79);
            this.TextPasswd2.Name = "TextPasswd2";
            this.TextPasswd2.Size = new System.Drawing.Size(115, 15);
            this.TextPasswd2.TabIndex = 1;
            this.TextPasswd2.Text = "Повторите пароль:";
            // 
            // Info1
            // 
            this.Info1.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Bold);
            this.Info1.Location = new System.Drawing.Point(15, 9);
            this.Info1.Name = "Info1";
            this.Info1.Size = new System.Drawing.Size(288, 23);
            this.Info1.TabIndex = 2;
            this.Info1.Text = "Создание мастер-пароля";
            this.Info1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // SetPasswd1
            // 
            this.SetPasswd1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.SetPasswd1.Location = new System.Drawing.Point(133, 44);
            this.SetPasswd1.MaxLength = 64;
            this.SetPasswd1.Name = "SetPasswd1";
            this.SetPasswd1.PasswordChar = '*';
            this.SetPasswd1.Size = new System.Drawing.Size(165, 22);
            this.SetPasswd1.TabIndex = 3;
            // 
            // SetPasswd2
            // 
            this.SetPasswd2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.SetPasswd2.Location = new System.Drawing.Point(133, 75);
            this.SetPasswd2.MaxLength = 64;
            this.SetPasswd2.Name = "SetPasswd2";
            this.SetPasswd2.PasswordChar = '*';
            this.SetPasswd2.Size = new System.Drawing.Size(165, 22);
            this.SetPasswd2.TabIndex = 4;
            // 
            // btnSave
            // 
            this.btnSave.Location = new System.Drawing.Point(15, 110);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(100, 29);
            this.btnSave.TabIndex = 5;
            this.btnSave.Text = "Сохранить";
            this.btnSave.UseVisualStyleBackColor = true;
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Location = new System.Drawing.Point(198, 110);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(100, 29);
            this.btnCancel.TabIndex = 6;
            this.btnCancel.Text = "Отмена";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // SetMasterPasswd
            // 
            this.AcceptButton = this.btnSave;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.Tan;
            this.ClientSize = new System.Drawing.Size(312, 146);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnSave);
            this.Controls.Add(this.SetPasswd2);
            this.Controls.Add(this.SetPasswd1);
            this.Controls.Add(this.Info1);
            this.Controls.Add(this.TextPasswd2);
            this.Controls.Add(this.TextPasswd1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SetMasterPasswd";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Создание мастер-пароля - Admin DAVrun";
            this.Load += new System.EventHandler(this.Form5_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label TextPasswd1;
        private System.Windows.Forms.Label TextPasswd2;
        private System.Windows.Forms.Label Info1;
        private System.Windows.Forms.TextBox SetPasswd1;
        private System.Windows.Forms.TextBox SetPasswd2;
        private System.Windows.Forms.Button btnSave;
        private System.Windows.Forms.Button btnCancel;
    }
}