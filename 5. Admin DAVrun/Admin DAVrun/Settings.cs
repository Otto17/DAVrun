using System;               // Библиотека предоставляет доступ к базовым классам и функциональности .NET Framework
using System.Windows.Forms; // Библиотека используется для создания графического пользовательского интерфейса (GUI) в приложениях Windows

namespace Admin_DAVrun
{
    public partial class Settings : Form
    {
        private bool isCheckBoxSaveChecked; // Флаг для изменения состояния кнопки "Да" в зависимости от состояния галочки "checkBoxSave"

        internal Settings(bool isCheckBoxSaveChecked)   // Получаем состояние флага от функции "btnSettings_Click()" из формы "Connect"
        {
            InitializeComponent();  // Инициализируем компоненты формы

            this.isCheckBoxSaveChecked = isCheckBoxSaveChecked; // Меняем состояние кнопки в зависимости от того, задан мастер-пароль или нет
        }

        private void Form4_Load(object sender, EventArgs e)
        {
            this.FormBorderStyle = FormBorderStyle.FixedSingle; // Запрещаем менять размер окна мышкой (для формы с настройками)
            UpdateButtonState();                                // Обновляем состояние флага для кнопки "Да"

            //Заполняем в текстовые поля значения из конфига
            //Имена групп
            NameGroup1.Text = Connect.config.Group1;
            NameGroup2.Text = Connect.config.Group2;
            NameGroup3.Text = Connect.config.Group3;
            NameGroup4.Text = Connect.config.Group4;

            //Имена подгрупп
            SubNameGroup1.Text = Connect.config.SubGroup1;
            SubNameGroup2.Text = Connect.config.SubGroup2;
            SubNameGroup3.Text = Connect.config.SubGroup3;
            SubNameGroup4.Text = Connect.config.SubGroup4;
            SubNameGroup5.Text = Connect.config.SubGroup5;
            SubNameGroup6.Text = Connect.config.SubGroup6;
            SubNameGroup7.Text = Connect.config.SubGroup7;

            //Состояния чекбоксов
            OnOffGroup2.Checked = Connect.config.OnOffGroup2;
            OnOffGroup3.Checked = Connect.config.OnOffGroup3;
            OnOffGroup4.Checked = Connect.config.OnOffGroup4;

            OnOffSubGroup2.Checked = Connect.config.OnOffSubGroup2;
            OnOffSubGroup3.Checked = Connect.config.OnOffSubGroup3;
            OnOffSubGroup4.Checked = Connect.config.OnOffSubGroup4;
            OnOffSubGroup5.Checked = Connect.config.OnOffSubGroup5;
            OnOffSubGroup6.Checked = Connect.config.OnOffSubGroup6;
            OnOffSubGroup7.Checked = Connect.config.OnOffSubGroup7;

            //Названия каталогов на Linux сервере для групп
            LinkPathGroup1.Text = Connect.config.LinkGroup1;
            LinkPathGroup2.Text = Connect.config.LinkGroup2;
            LinkPathGroup3.Text = Connect.config.LinkGroup3;
            LinkPathGroup4.Text = Connect.config.LinkGroup4;

            //Названия каталогов на Linux сервере для подгрупп
            LinkPathSubGroup1.Text = Connect.config.LinkSubGroup1;
            LinkPathSubGroup2.Text = Connect.config.LinkSubGroup2;
            LinkPathSubGroup3.Text = Connect.config.LinkSubGroup3;
            LinkPathSubGroup4.Text = Connect.config.LinkSubGroup4;
            LinkPathSubGroup5.Text = Connect.config.LinkSubGroup5;
            LinkPathSubGroup6.Text = Connect.config.LinkSubGroup6;
            LinkPathSubGroup7.Text = Connect.config.LinkSubGroup7;

            //Названия каталогов для установочного файла и файлов с именами ПК
            LinkPathSetup.Text = Connect.config.LinkSetup;
            LinkPathInstall.Text = Connect.config.LinkInstall;

            //Полный путь до каталогов с названиями
            LinkCommonPath.Text = Connect.config.LinkCommonPath;
        }


        //Обновление состояния кнопки "Да"
        private void UpdateButtonState()
        {
            btnChangePasswd.Enabled = isCheckBoxSaveChecked;    // Включаем или выключаем кнопку
        }

        //Кнопка открытия окна для изменения мастер-пароля
        private void btnChangePasswd_Click(object sender, EventArgs e)
        {
            SetMasterPasswd SetMasterPasswd = new SetMasterPasswd();  // Создаём экземпляр формы
            SetMasterPasswd.ShowDialog();                             // Отображаем форму и делаем окно активным, запрещая переключение на предыдущее окно
        }

        //Кнопка для сохранения имён групп и подгрупп, а так же их Вкл./Выкл. в конфиг
        private void btnSaveGroup_Click(object sender, EventArgs e)
        {
            //Устанавливаем новые значения из текстовых полей в конфиг
            //Для групп
            Connect.config.Group1 = NameGroup1.Text;
            Connect.config.Group2 = NameGroup2.Text;
            Connect.config.Group3 = NameGroup3.Text;
            Connect.config.Group4 = NameGroup4.Text;

            //Для подгрупп
            Connect.config.SubGroup1 = SubNameGroup1.Text;
            Connect.config.SubGroup2 = SubNameGroup2.Text;
            Connect.config.SubGroup3 = SubNameGroup3.Text;
            Connect.config.SubGroup4 = SubNameGroup4.Text;
            Connect.config.SubGroup5 = SubNameGroup5.Text;
            Connect.config.SubGroup6 = SubNameGroup6.Text;
            Connect.config.SubGroup7 = SubNameGroup7.Text;

            //Сохраняем состояния чекбоксов для групп (Вкл./Выкл.)
            Connect.config.OnOffGroup2 = OnOffGroup2.Checked;
            Connect.config.OnOffGroup3 = OnOffGroup3.Checked;
            Connect.config.OnOffGroup4 = OnOffGroup4.Checked;

            //Сохраняем состояния чекбоксов для подгрупп (Вкл./Выкл.)
            Connect.config.OnOffSubGroup2 = OnOffSubGroup2.Checked;
            Connect.config.OnOffSubGroup3 = OnOffSubGroup3.Checked;
            Connect.config.OnOffSubGroup4 = OnOffSubGroup4.Checked;
            Connect.config.OnOffSubGroup5 = OnOffSubGroup5.Checked;
            Connect.config.OnOffSubGroup6 = OnOffSubGroup6.Checked;
            Connect.config.OnOffSubGroup7 = OnOffSubGroup7.Checked;

            //Сохраняем в конфиг
            Connect ConnectForm = new Connect();
            ConnectForm.SaveConfig();

            //Обновляем текстовые значения и состояния элементов на форме "Management"
            if (Connect.formManagementInstance != null) // Если экземпляр формы существует
            {
                Connect.formManagementInstance.SetGroupNames(Connect.config);   // Вызываем метод "SetGroupNames", передавая ему объект конфигурации "config"
            }

            MessageBox.Show("Сохранено в конфиг!");
        }

        //Кнопка для сохранения названий каталогов на Linux сервере в конфиг
        private void btnSaveLink_Click(object sender, EventArgs e)
        {
            //Устанавливаем новые значения из текстовых полей в конфиг
            //Для групп
            Connect.config.LinkGroup1 = LinkPathGroup1.Text;
            Connect.config.LinkGroup2 = LinkPathGroup2.Text;
            Connect.config.LinkGroup3 = LinkPathGroup3.Text;
            Connect.config.LinkGroup4 = LinkPathGroup4.Text;

            //Для подгрупп
            Connect.config.LinkSubGroup1 = LinkPathSubGroup1.Text;
            Connect.config.LinkSubGroup2 = LinkPathSubGroup2.Text;
            Connect.config.LinkSubGroup3 = LinkPathSubGroup3.Text;
            Connect.config.LinkSubGroup4 = LinkPathSubGroup4.Text;
            Connect.config.LinkSubGroup5 = LinkPathSubGroup5.Text;
            Connect.config.LinkSubGroup6 = LinkPathSubGroup6.Text;
            Connect.config.LinkSubGroup7 = LinkPathSubGroup7.Text;

            //Названия каталогов для установочного файла и файлов с именами ПК
            Connect.config.LinkSetup = LinkPathSetup.Text;
            Connect.config.LinkInstall = LinkPathInstall.Text;
            Connect.config.LinkCommonPath = LinkCommonPath.Text;

            // Сохраняем в конфиг
            Connect ConnectForm = new Connect();
            ConnectForm.SaveConfig();

            MessageBox.Show("Сохранено в конфиг!");
        }

        //Метод для показа PUSH уведомления
        private void ShowNotification(string message, Control control, int displayTimeInSeconds)    // Текст сообщения, элемент управления от которого показывать сообщение, время показа в миллисекундах
        {
            ToolTip toolTip = new ToolTip();
            toolTip.Show(message, control, 0, 0, displayTimeInSeconds); // Уведомление будет показано "displayTimeInSeconds" миллисекунд
        }

        //Вывод PUSH уведомления при клике мышкой на строку "Имя группы #1:"
        private void labelName1_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(labelName1.Text)) // Проверяем, не пустой ли текст в "labelName1"
            {
                ShowNotification("Эту группу отключить нельзя (должна быть хотя бы одна активная группа)", labelName1, 4000);   // Выводим текст на 4 секунды
            }
        }

        //Вывод PUSH уведомления при клике мышкой на строку "Имя группы #4:"
        private void labelName4_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(labelName4.Text)) // Проверяем, не пустой ли текст в "labelName4"
            {
                ShowNotification("Эта группа индивидуальна и не имеет подгрупп", labelName4, 3000); // Выводим текст на 3 секунды
            }
        }

        //Вывод PUSH уведомления при клике мышкой на строку "Имя подгруппы 1:"
        private void labelSubName1_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(labelSubName1.Text))  // Проверяем, не пустой ли текст в "labelSubName1"
            {
                ShowNotification("Эту подгруппу отключить нельзя (должна быть хотя бы одна активная подгруппа)", labelSubName1, 4000);  // Выводим текст на 4 секунды
            }
        }

        //Вывод PUSH уведомления при клике мышкой на строку "Полный путь до каталогов с началом имён:"
        private void labelLinkPathCommon_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(labelLinkPathCommon.Text))    // Проверяем, не пустой ли текст в "labelLinkPathCommon"
            {
                ShowNotification("В конце пути правый слеш '/' НЕ ставить!", labelLinkPathCommon, 3000);    // Выводим текст на 3 секунды
            }
        }
    }
}
