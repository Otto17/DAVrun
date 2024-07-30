using System;               // Библиотека предоставляет доступ к базовым классам и функциональности .NET Framework
using System.Linq;          // Библиотека предоставляет методы для выполнения операций LINQ (Language-Integrated Query) над данными
using System.Windows.Forms; // Библиотека используется для создания графического пользовательского интерфейса (GUI) в приложениях Windows

namespace Admin_DAVrun
{
    public partial class SetMasterPasswd : Form
    {
        private const int CP_NOCLOSE_BUTTON = 0x200;    // Константа для блокировки кнопки "крестик" на форме (512 в десятичной системе)
        internal bool IsPasswordSet { get; private set; } = false;  // Переменная для установки мастер-пароля

        internal SetMasterPasswd()
        {
            InitializeComponent();  // Инициализируем компоненты формы
        }

        private void Form5_Load(object sender, EventArgs e)
        {
            this.FormBorderStyle = FormBorderStyle.FixedSingle; // Задаём фиксированный размер окна, без возможности менять его размер мышкой
        }

        //Блокировка кнопки "крестик" на форме
        protected override CreateParams CreateParams    // Создаём защищённое переопределение
        {
            get
            {
                CreateParams myCp = base.CreateParams;                  // Создаём параметры "myCp" и инициализируем базовыми параметрами
                myCp.ClassStyle = myCp.ClassStyle | CP_NOCLOSE_BUTTON;  // Добавляем к параметру "ClassStyle" значение константы через побитовое "или"
                return myCp;                                            // Возвращаем изменённый объект параметров "myCp"
            }
        }

        //Кнопка сохранения мастер-пароля
        private void btnSave_Click(object sender, EventArgs e)
        {
            if (SetPasswd1.Text == SetPasswd2.Text) // Проверка пароля на корректность в обеих полях
            {
                Connect.config.MasterPassword = SetPasswd1.Text;                                // Сохраняем пароль в конфиг из первой текстовой строки
                Connect.config.PasswordAttempts = Connect.PassAttempts;                         // Устанавливаем кол-во попыток ввода пароля
                Connect.config.Save = true;                                                     // Поднимаем флаг, что данные в конфиге сохранены
                var form1Instance = Application.OpenForms.OfType<Connect>().FirstOrDefault();   // Находим экземпляр формы "Connect" среди открытых форм типа "Connect" или равным null, если такой формы нет
                form1Instance?.SaveConfig();                                                    // Вызываем метод для сохранения конфиг файла
                IsPasswordSet = true;                                                           // Поднимаем флаг, что мастер-пароль установлен
                this.Close();                                                                   // Закрываем форму "SetMasterPasswd"
            }
            else
            {
                MessageBox.Show("Пароли не совпадают. Попробуйте еще раз.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        //Кнопка отмены
        private void btnCancel_Click(object sender, EventArgs e)
        {
            var result = MessageBox.Show("Если не создать мастер-пароль, сохранить конфиг будет нельзя.", "Предупреждение", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

            if (result == DialogResult.Yes) // Проверяем, была ли нажата кнопка "Да"
            {
                IsPasswordSet = false;  // Опускаем флаг, что бы не дать установить галочку на "checkBoxSave" на форме "Connect"
                this.Close();           // Закрываем форму "SetMasterPasswd"
            }
        }
    }
}
