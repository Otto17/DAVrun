using System;                       // Библиотека предоставляет доступ к базовым классам и функциональности .NET Framework
using System.IO;                    // Библиотека отвечает за ввод и вывод данных, включая чтение и запись файлов
using System.Drawing;               // Библиотека используется для работы с изображениями, включая их загрузку, создание и обработку
using System.Linq;                  // Библиотека предоставляет методы для выполнения операций LINQ (Language-Integrated Query) над данными
using System.Threading.Tasks;       // Библиотека предоставляет средства для работы с асинхронными задачами и потоками
using System.Windows.Forms;         // Библиотека используется для создания графического пользовательского интерфейса (GUI) в приложениях Windows
using System.Security.Cryptography; // Библиотека содержит классы для работы с криптографическими операциями, такими как шифрование и хеширование
using Renci.SshNet;                 // Библиотека предоставляет возможность установления SSH-соединения и работы с удаленными серверами (необходимо добавить из NuGet)
using Newtonsoft.Json;              // Библиотека позволяет работать с форматом JSON, включая чтение, запись и обработку данных в формате JSON (необходимо добавить из NuGet)

namespace Client_DAVrun
{
    public partial class Connect : Form
    {
        private SshClient sshClient;                        // Для подключения к SSH
        private SftpClient sftpClient;                      // Для подключения к SFTP
        internal static Management formManagementInstance;  // Для доступа к форме "Management"
        private bool isPasswordVisible = false;             // Флаг для кнопки показать/скрыть пароль
        internal static Config config = new Config();       // Для доступа к классу с конфигом
        internal static int PassAttempts = 5;               // Кол-во попыток ввода мастер-пароля

        //Сохраняем конфиг в папку пользователя, от которого запустили программу, через метод "Environment.GetFolderPath()" для корректного получения пути к локальной папке пользователя
        internal static readonly string configFilePath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Client DAVrun", "config.json"); // Аналогично "%LocalAppData%"

        //Ключ шифрования конфига AES-256 и случайное значение IV (Initialization Vector - Вектор Инициализации)
        internal static readonly byte[] Key = new byte[] { 43, 129, 224, 13, 255, 200, 159, 43, 243, 140, 62, 81, 84, 32, 0, 231, 32, 168, 55, 41, 252, 7, 214, 40, 2, 187, 16, 103, 147, 21, 106, 106 }; // 32-х байтовый для AES-256 (именно БАЙТОВЫЙ, а не битный)
        internal static readonly byte[] IV = new byte[] { 26, 167, 13, 65, 162, 145, 139, 185, 55, 46, 24, 66, 107, 25, 3, 200 }; // 16-ти байтовый IV (именно БАЙТОВЫЙ, а не битный)

        internal Connect()
        {
            InitializeComponent();  // Инициализируем компоненты формы

            //Если при запуске формы папки "Client DAVrun" не существует по пути "%LocalAppData%", тогда создаём её
            string folderPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Client DAVrun");
            if (!Directory.Exists(folderPath))
            {
                Directory.CreateDirectory(folderPath);
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.FormBorderStyle = FormBorderStyle.FixedSingle; // Задаём фиксированный размер окна, без возможности менять его размер мышкой
            LoadConfig();                                       // Загружаем данные из конфига

            //Привязка метода "UpdateCheckBoxSaveState" к событиям изменения текста в текстовых полях подключения к SSH
            textBoxIP.TextChanged += (s, ev) => UpdateCheckBoxSaveState();
            textBoxPort.TextChanged += (s, ev) => UpdateCheckBoxSaveState();
            textBoxLogin.TextChanged += (s, ev) => UpdateCheckBoxSaveState();
            textBoxPasswd.TextChanged += (s, ev) => UpdateCheckBoxSaveState();

            UpdateCheckBoxSaveState();  // Проверяем начальное состояние "checkBoxSave"

            //Если мастер-пароль установлен, показываем форму для ввода пароля
            if (!string.IsNullOrEmpty(config.MasterPassword))
            {
                this.Hide();                                    // Скрываем форму "Connect"
                MasterPasswd MasterPasswd = new MasterPasswd(); // Создаём новый объект формы "MasterPasswd"
                MasterPasswd.ShowDialog();                      // Открываем форму в диалоговом режиме для блокировки других действий, пока форма открыта

                if (!MasterPasswd.IsPasswordCorrect)    // Проверка введённого мастер-пароля на корректность
                {
                    Application.Exit(); // Если не верно, то завершаем свою работу после исчерпания кол-ва попыток ввода мастер-пароля
                }
                else
                {
                    config.PasswordAttempts = PassAttempts; // Восстанавливаем счетчик попыток
                    SaveConfig();                           // Сохраняем счётчик в конфиг
                    this.Show();                            // Отображаем скрытую форму
                }
            }

            //Если "formManagementInstance" уже существует, обновляем его названия
            if (formManagementInstance != null)
            {
                formManagementInstance.SetGroupNames(config);
            }
        }

        //Публичная функция для установки цвета статуса подключения по SSH
        internal void SetStatusConnectBackColor(Color color)
        {
            StatusConnect.BackColor = color;
        }

        //Публичная функция для вывода симвода статуса подключения к SSH
        internal void SetTextStatusConnect(string text)
        {
            TextStatusConnect.Text = text;
        }

        //Кнопка "Подключиться" к SSH и SFTP
        private async void btnConnect_Click_1(object sender, EventArgs e)
        {
            //Проверка, чтобы все поля были заполнены перед нажатием кнопки
            if (string.IsNullOrEmpty(textBoxIP.Text) || string.IsNullOrEmpty(textBoxPort.Text) ||
                string.IsNullOrEmpty(textBoxLogin.Text) || string.IsNullOrEmpty(textBoxPasswd.Text))
            {
                MessageBox.Show("Все поля должны быть заполнены.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            btnConnect.Enabled = false; // Деактивируем кнопку, пока идёт попытка подключения

            //Пишем в переменные значения с текстовых полей
            string ipAddress = textBoxIP.Text;
            int port = int.Parse(textBoxPort.Text);
            string username = textBoxLogin.Text;
            string password = textBoxPasswd.Text;

            try
            {
                //Создаём новые объекты
                sshClient = new SshClient(ipAddress, port, username, password);
                sftpClient = new SftpClient(ipAddress, port, username, password);

                //Запускаем асинхронную задачу с подключениями
                await Task.Run(() =>
                {
                    sshClient.Connect();
                    sftpClient.Connect();
                });

                if (sshClient.IsConnected && sftpClient.IsConnected)    // Проверка, что оба клиента подключены
                {
                    SetStatusConnectBackColor(Color.Green); // Меняем цвет формы "StatusConnect"
                    SetTextStatusConnect("✓");              // Показываем галочку
                    await Task.Delay(1000);                 // Ждём 1 сек. для наглядности

                    //Если "formManagementInstance" не существует или был уничтожен, то создаём новый экземпляр формы "Management"
                    if (formManagementInstance == null || formManagementInstance.IsDisposed)
                    {
                        formManagementInstance = new Management(this, sshClient, sftpClient);

                        //Обработчик событий для отключения SSH и SFTP с последующим завершением работы программы
                        formManagementInstance.FormClosed += (s, args) =>
                        {
                            sshClient.Disconnect();
                            sftpClient.Disconnect();
                            Application.Exit();
                        };
                    }

                    formManagementInstance.Show();  // Показываем форму "formManagementInstance"
                    this.Hide();                    // Скрываем текущюю форму
                }
                else
                {
                    throw new Exception();  // Если хотя бы один из клиентов (SSH или SFTP) не подключены, то генерируется исключение типа "Exception"
                }
            }
            catch (Exception)
            {
                SetStatusConnectBackColor(Color.Red);   // Меняем цвет формы "StatusConnect"
                SetTextStatusConnect("X");              // Показываем X

                // Проверка на null и вызов метода Disconnect
                if (sshClient != null && sshClient.IsConnected)
                {
                    sshClient.Disconnect();
                }

                if (sftpClient != null && sftpClient.IsConnected)
                {
                    sftpClient.Disconnect();
                }
            }
            finally
            {
                btnConnect.Enabled = true;  // Гарантированно активируем кнопку независимо от результата подключения
            }
        }

        //Кнопка с глазом (показать/скрыть пароль)
        private void btnOpenClocePasswd_Click(object sender, EventArgs e)
        {
            if (isPasswordVisible) // Если пароль скрыт
            {
                textBoxPasswd.PasswordChar = '*';                            // Ставим маску
                btnOpenClocePasswd.Image = Properties.Resources.ClosePasswd; // Меняем картинку с именем "ClosePasswd" (перечёркнутый глаз)
            }
            else // Если пароль показан
            {
                textBoxPasswd.PasswordChar = '\0';                          // Убираем маску
                btnOpenClocePasswd.Image = Properties.Resources.OpenPasswd; // Меняем картинку с именем "OpenPasswd" (открытый глаз)
            }
            isPasswordVisible = !isPasswordVisible; // Меняем флаг местами
        }

        //Кнопка с шестерёнкой (открывает форму с настройки)
        private void btnSettings_Click(object sender, EventArgs e)
        {
            Settings Settings = new Settings(checkBoxSave.Checked); // Передаем текущее состояние "checkBoxSave" в форму
            Settings.ShowDialog();                                  // Отображаем форму и делаем окно активным, запрещая переключение на предыдущее окно
        }

        //Галочка "Запомнить"
        private void checkBoxSave_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBoxSave.Checked)   // Если галочка стоит, тогда сохраняем значения полей в конфиг
            {
                config.IP = textBoxIP.Text;
                config.Port = int.Parse(textBoxPort.Text);
                config.Login = textBoxLogin.Text;
                config.Password = textBoxPasswd.Text;
                config.Save = checkBoxSave.Checked;

                if (string.IsNullOrEmpty(config.MasterPassword))    // Если поле с мастер-паролем пустое
                {
                    //Показываем окно для установки мастер-пароля (только при ручной установке галочки)
                    SetMasterPasswd SetMasterPasswd = new SetMasterPasswd();
                    SetMasterPasswd.ShowDialog();

                    if (!SetMasterPasswd.IsPasswordSet) // Если мастер-пароль не установлен
                    {
                        checkBoxSave.Checked = false;   // Снимаем галочку с "Запомнить"
                    }
                }
                SaveConfig();   // Сохраняем значения полей в конфиг
            }
            else
            {
                config.MasterPassword = string.Empty; // Если поле мастер-пароля пустое, то присваиваем пустые значения полям конфига
                ClearConfig();                        // Очищаем конфиг
            }
        }

        //Метод для проверки состояния текстовых полей на форме в реальном времени, если хоть одно не заполнено, тогда чекбокс "checkBoxSave" будет не активен
        private void UpdateCheckBoxSaveState()
        {
            checkBoxSave.Enabled =
                !string.IsNullOrEmpty(textBoxIP.Text) &&
                !string.IsNullOrEmpty(textBoxPort.Text) &&
                !string.IsNullOrEmpty(textBoxLogin.Text) &&
                !string.IsNullOrEmpty(textBoxPasswd.Text);
        }

        //Разрешаем в поле "Порт" вводить только цифры
        private void textBoxPort_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar)) // Проверяем, что нажатая клавиша не является управляющим символом и не является числом
            {
                e.Handled = true; // Если это так, то отменяем ввод символа
            }
        }

        //Разрешаем в поле "Порт" вводить цифры не более 65535
        private void textBoxPort_TextChanged(object sender, EventArgs e)
        {
            if (int.TryParse(textBoxPort.Text, out int number) && number > 65535)   //Преобразовываем введённые цифры из текста в число и сверяем, если число больше 65535
            {
                textBoxPort.Text = "65535"; // Устанавливаем это число как максимальное
            }
        }

        //Меняем введённые запятые на точки для поля "IP адрес/Домен"
        private void textBoxIP_TextChanged(object sender, EventArgs e)
        {
            if (textBoxIP.Text.Contains(','))   // Проверяем наличие запятой во введённом тексте
            {
                textBoxIP.Text = textBoxIP.Text.Replace(',', '.');  // Заменяем запятую на точку
                textBoxIP.SelectionStart = textBoxIP.Text.Length;   // Перемещаем курсор в конец строки
            }
        }

        //Функция загрузки конфига из JSON файла
        private void LoadConfig()
        {
            if (File.Exists(configFilePath))    // Проверка наличия конфига по указанному пути
            {
                try
                {
                    var encryptedConfigJson = File.ReadAllBytes(configFilePath);    // Считываем массив байтов из конфига
                    var configJson = Decrypt(encryptedConfigJson, Key, IV);         // Дешифруем
                    config = JsonConvert.DeserializeObject<Config>(configJson);     // Преобразовываем в объект "config" десериализацию

                    //Проверка, если IP, Логин и Пароль не пустые и пустой мастер-пароль
                    if ((!string.IsNullOrEmpty(config.IP) || !string.IsNullOrEmpty(config.Login) || !string.IsNullOrEmpty(config.Password)) && string.IsNullOrEmpty(config.MasterPassword))
                    {
                        CreateDefaultConfig();  // Создаём конфиг по шаблону
                    }
                    else // Иначе заполняем текстовые поля значениями из конфига
                    {
                        textBoxIP.Text = config.IP;
                        textBoxPort.Text = config.Port.ToString();
                        textBoxLogin.Text = config.Login;
                        textBoxPasswd.Text = config.Password;
                        checkBoxSave.Checked = config.Save;
                    }

                    if (formManagementInstance != null) // Если экземпляр формы существует
                    {
                        formManagementInstance.SetGroupNames(config);   // Вызываем метод "SetGroupNames", передавая ему объект конфигурации "config"
                    }
                }
                catch (Exception)   // Если происходит любое другое исключение
                {
                    File.Delete(configFilePath);    // Удаляем конфиг
                    CreateDefaultConfig();          // И создаём конфиг по шаблону
                }
            }
            else
            {
                CreateDefaultConfig();  // Если конфиг не существует, то создаём конфиг по шаблону
            }
        }

        //Функция сохранения конфиг файла
        internal void SaveConfig()
        {
            var configJson = JsonConvert.SerializeObject(config);       // Преобразуем объект "config" в формат JSON
            var encryptedConfigJson = Encrypt(configJson, Key, IV);     // Зашифровываем конфиг
            File.WriteAllBytes(configFilePath, encryptedConfigJson);    // Записываем зашифрованный JSON в файл по указанному пути "configFilePath"
        }

        //Функция очистки конфиг файла
        private void ClearConfig()
        {
            var config = new Config();
            var configJson = JsonConvert.SerializeObject(config);       // Преобразуем объект "config" в формат JSON
            var encryptedConfigJson = Encrypt(configJson, Key, IV);     // Зашифровываем конфиг
            File.WriteAllBytes(configFilePath, encryptedConfigJson);    // Записываем зашифрованный JSON в файл по указанному пути "configFilePath"
        }

        //Функция создания конфига по шаблону
        private void CreateDefaultConfig()
        {
            var defaultConfig = new Config();
            var configJson = JsonConvert.SerializeObject(defaultConfig);    // Преобразуем объект "config" в формат JSON
            var encryptedConfigJson = Encrypt(configJson, Key, IV);         // Зашифровываем конфиг
            File.WriteAllBytes(configFilePath, encryptedConfigJson);        // Записываем зашифрованный JSON в файл по указанному пути "configFilePath"
        }

        //Функция шифрования конфига по алгоритму AES-256 и IV (Вектор Инициализации)
        private static byte[] Encrypt(string plainText, byte[] key, byte[] iv)  // Строка шифрования, ключ и Вектор Инициализации для шифрования
        {
            using (Aes aes = Aes.Create())  // Создаём экземпляр класса
            {
                //Устанавливаем свойства "Key" и "IV" у объекта "aes" равными переданным в метод параметрам "key" и "iv"
                aes.Key = key;
                aes.IV = iv;
                ICryptoTransform encryptor = aes.CreateEncryptor(aes.Key, aes.IV);  // Создаём объект, который будет выполнять шифрование

                using (MemoryStream ms = new MemoryStream())    // Создаём объект "MemoryStream ms" для хранения зашифрованных данных
                {
                    using (CryptoStream cs = new CryptoStream(ms, encryptor, CryptoStreamMode.Write))   // Создаём объект "CryptoStream cs", который применяет алгоритм шифрования "encryptor" к потоку "ms" в режиме записи
                    {
                        using (StreamWriter sw = new StreamWriter(cs))  // Создаём объект "StreamWriter sw", который будет записывать данные в строку "plainText" в объект "cs"
                        {
                            sw.Write(plainText);    // Сохраняем строку "plainText" в "sw"
                        }
                        return ms.ToArray();    // Возвращаем зашифрованные данные в виде массива байт
                    }
                }
            }
        }

        //Функция дешифрования конфига по алгоритму AES-256 и IV (Вектор Инициализации)
        private static string Decrypt(byte[] cipherText, byte[] key, byte[] iv) // Строка дешифрования, ключ и Вектор Инициализации для шифрования
        {
            using (Aes aes = Aes.Create())  // Создаём экземпляр класса
            {
                //Устанавливаем свойства "Key" и "IV" у объекта "aes" равными переданным в метод параметрам "key" и "iv"
                aes.Key = key;
                aes.IV = iv;
                ICryptoTransform decryptor = aes.CreateDecryptor(aes.Key, aes.IV);  // Создаём объект, который будет выполнять дешифрование

                using (MemoryStream ms = new MemoryStream(cipherText))  // Создаём объект "MemoryStream ms" для хранения расшифрованных данных
                {
                    using (CryptoStream cs = new CryptoStream(ms, decryptor, CryptoStreamMode.Read))    // Создаём объект "CryptoStream cs", который применяет алгоритм дешифрования "decryptor" к потоку "ms" в режиме чтения
                    {
                        using (StreamReader sr = new StreamReader(cs))  // Создаём объект "StreamWriter sr", который будет считывать данные из "CryptoStream cs"
                        {
                            return sr.ReadToEnd();  // Читаем расшифрованные данные и возвращаем их в виде строки
                        }
                    }
                }
            }
        }
    }

    //Класс с шаблоном конфиг файла
    public class Config
    {
        public string IP { get; set; } = ""; // IP адрес или Домен
        public int Port { get; set; } = 22; // 22 порт SSH по умолчанию
        public string Login { get; set; } = ""; // Логин
        public string Password { get; set; } = ""; // Пароль
        public bool Save { get; set; } = false; // Состояние сохранения конфига
        public string MasterPassword { get; set; } = ""; // Мастер-пароль
        public int PasswordAttempts { get; set; } = Connect.PassAttempts; // Количество попыток ввода пароля

        //Названия Групп (на форме с настройками)
        public string Group1 { get; set; } = "Омск";
        public string Group2 { get; set; } = "Область";
        public string Group3 { get; set; } = "Тюмень";
        public string Group4 { get; set; } = "Офис";

        //Названия Подгрупп (на форме с настройками)
        public string SubGroup1 { get; set; } = "Теле2";
        public string SubGroup2 { get; set; } = "Изюм";
        public string SubGroup3 { get; set; } = "Мегафон";
        public string SubGroup4 { get; set; } = "МТС";
        public string SubGroup5 { get; set; } = "Билайн";
        public string SubGroup6 { get; set; } = "Xiaomi";
        public string SubGroup7 { get; set; } = "Прочие";

        //Состояния групп и подгрупп (Включены или Выключены на форме с настройками)
        public bool OnOffGroup2 { get; set; } = true;
        public bool OnOffGroup3 { get; set; } = true;
        public bool OnOffGroup4 { get; set; } = true;
        public bool OnOffSubGroup2 { get; set; } = true;
        public bool OnOffSubGroup3 { get; set; } = true;
        public bool OnOffSubGroup4 { get; set; } = true;
        public bool OnOffSubGroup5 { get; set; } = true;
        public bool OnOffSubGroup6 { get; set; } = true;
        public bool OnOffSubGroup7 { get; set; } = true;

        //Поля с именами каталогов и полного пути до них на Linux сервере (на форме с настройками)
        public string LinkGroup1 { get; set; } = "Omsk";
        public string LinkGroup2 { get; set; } = "Oblast";
        public string LinkGroup3 { get; set; } = "Tumen";
        public string LinkGroup4 { get; set; } = "Office";

        public string LinkSubGroup1 { get; set; } = "Tele2";
        public string LinkSubGroup2 { get; set; } = "Izum";
        public string LinkSubGroup3 { get; set; } = "Megafon";
        public string LinkSubGroup4 { get; set; } = "MTS";
        public string LinkSubGroup5 { get; set; } = "Beeline";
        public string LinkSubGroup6 { get; set; } = "Xiaomi";
        public string LinkSubGroup7 { get; set; } = "Other";

        public string LinkSetup { get; set; } = "Setup";
        public string LinkInstall { get; set; } = "Points";
        public string LinkCommonPath { get; set; } = "/var/www/webdav";
    }
}
