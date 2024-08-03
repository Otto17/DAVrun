using System;               // Библиотека предоставляет доступ к базовым классам и функциональности .NET Framework
using System.Windows.Forms; // Библиотека используется для создания графического пользовательского интерфейса (GUI) в приложениях Windows


namespace Admin_DAVrun
{
    public partial class About : Form
    {
        public About()
        {
            InitializeComponent();  // Инициализируем компоненты формы
        }

        private void Form3_O_Programme_Load(object sender, EventArgs e)
        {
            this.FormBorderStyle = FormBorderStyle.FixedSingle; // Задаём фиксированный размер окна, без возможности менять его размер мышкой
        }

        private void richTextBox1_LinkClicked(object sender, LinkClickedEventArgs e)
        {
            System.Diagnostics.Process.Start(e.LinkText);   // Разрешаем открывать ссылки из текстового поля "О программе"
        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
