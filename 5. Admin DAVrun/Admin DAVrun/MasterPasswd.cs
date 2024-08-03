using System;               // Библиотека предоставляет доступ к базовым классам и функциональности .NET Framework
using System.Windows.Forms; // Библиотека используется для создания графического пользовательского интерфейса (GUI) в приложениях Windows


namespace Admin_DAVrun
{
    public partial class MasterPasswd : Form
    {
        internal bool IsPasswordCorrect { get; private set; } = false; // Переменная для проверки введённого мастер-пароля

        internal MasterPasswd()
        {
            InitializeComponent();  // Инициализируем компоненты формы
        }

        private void Form6_Load(object sender, EventArgs e)
        {
            this.FormBorderStyle = FormBorderStyle.FixedSingle; // Задаём фиксированный размер окна, без возможности менять его размер мышкой
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            Connect Connect = new(); // Создаём экземпляр "Connect" класса "Connect"

            if (TextMaster.Text == Connect.config.MasterPassword)   // Проверка мастер-пароля на корректность
            {
                IsPasswordCorrect = true;                               // Поднимаем флаг ввода корректного мастер-пароля
                Connect.config.PasswordAttempts = Connect.PassAttempts; // Сбрасываем попытки
                Connect.SaveConfig();                                   // Сохраняем сброшенные попытки в конфиг
                this.Close();                                           // Закрываем форму "MasterPasswd"
            }
            else
            {
                Connect.config.PasswordAttempts--;  // Уменьшаем счётчик попыток ввода пароля на -1
                Connect.SaveConfig();               // Сохраняем изменённый счётчик в конфиг

                if (Connect.config.PasswordAttempts <= 0)   // Если попытки пароля закончились
                {
                    MessageBox.Show("Попытки ввода закончились. Конфигурационный файл будет удален.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    System.IO.File.Delete(Connect.configFilePath);  // Удаляем конфиг файл
                    Application.Exit();                             // Завершаем работу приложения
                }
                else
                {
                    MessageBox.Show($"Неверный пароль. Осталось попыток: {Connect.config.PasswordAttempts}", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    TextMaster.SelectAll(); // Выделяем весь текст в поле ввода
                    TextMaster.Focus();     // Устанавливаем фокус на это же поле ввода
                }
            }
        }
    }
}