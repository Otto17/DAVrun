using System;                       // Библиотека предоставляет доступ к базовым классам и функциональности .NET Framework
using System.IO;                    // Библиотека отвечает за ввод и вывод данных, включая чтение и запись файлов
using System.Drawing;               // Библиотека используется для работы с изображениями, включая их загрузку, создание и обработку
using System.Linq;                  // Библиотека предоставляет методы для выполнения операций LINQ (Language-Integrated Query) над данными
using System.Threading.Tasks;       // Библиотека предоставляет средства для работы с асинхронными задачами и потоками
using System.Windows.Forms;         // Библиотека используется для создания графического пользовательского интерфейса (GUI) в приложениях Windows
using System.Security.Cryptography; // Библиотека содержит классы для работы с криптографическими операциями, такими как шифрование и хеширование
using System.Collections.Generic;   // Библиотека предоставляет обобщенные коллекции, такие как списки, словари, очереди и другие, для хранения и управления данными
using Renci.SshNet;                 // Библиотека предоставляет возможность установления SSH-соединения и работы с удаленными серверами (необходимо добавить из NuGet)
using System.Data;                  // Библиотека  предоставляет доступ к работе с данными и базами данных, включая подключение, извлечение и обновление данных
using System.Text;                  // Библиотека предоставляет классы и методы для работы с текстом, включая работу со строками, преобразованием кодировок и форматированием текста


namespace Admin_DAVrun
{
    public partial class Management : Form
    {
        private static readonly List<string> draggedFiles = [];     // Регистрация событий изменения состояния панели Drag and Drop
        private static readonly List<CheckBox> fileCheckBoxes = []; // Регистрация событий изменения состояния радио-кнопок для групп и подгрупп

        private string selectedFilePath;    // Переменная для работы с setup файлом (самораспаковывающийся архив)
        private string selectedFileHash;    // Переменная для работы с хеш setup файла

        private readonly SshClient sshClient;   // Создаём переменную (без возможности изменения) для SSH
        private readonly SftpClient sftpClient; // Создаём переменную (без возможности изменения) для SFTP

        private System.Timers.Timer autoUpdateTimer;    // Таймер для автообновления списка ожидания на установку

        internal Management(SshClient sshClient, SftpClient sftpClient)
        {
            InitializeComponent();          // Инициализируем компоненты формы
            InitializeAutoUpdateControls(); // Инициализация автообновления списка ожидания на установку

            //Устанавливаем значения полей "this.sshClient" и "this.sftpClient" равными переданным объектам "sshClient" и "sftpClient"
            this.sshClient = sshClient;
            this.sftpClient = sftpClient;

            labelhash.Click += labelhash_Click; // Подписка на событие клика для "labelhash" (для копирования значения хеша из поля)

            panelDrop.AllowDrop = true;                 // Разрешаем панели принимать перетаскиваемые элементы
            panelDrop.DragEnter += panel4_DragEnter;    // Привязываем обработчики событий...
            panelDrop.DragLeave += panel4_DragLeave;
            panelDrop.DragDrop += panel4_DragDrop;
            labelDrag.Click += panel4_Click;            // Привязываем click событие к новому обработчику
        }

        //Кнопка (меню) вызова формы с подключением по SSH (при нажатии на которую открывается форма "Connect", а форма "Management" скрывается)
        private void подключитсяПоSSHToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Connect Connect = Application.OpenForms.OfType<Connect>().FirstOrDefault(); // Находим экземпляр формы "Connect" среди открытых форм типа "Connect" или равным null, если такой формы нет

            if (Connect == null)    // Если не нашли форму "Connect", тогда создаём новый экземпляр формы "Connect"
            {
                Connect = new Connect();
            }
                        
            if (sshClient.IsConnected) sshClient.Disconnect();  // Отключаемся от SSH, если были подключены

            Connect.SetStatusConnectBackColor(SystemColors.ControlLight);   // Меняем цвет панели статуса подключения по SSH (как по умолчанию)
            Connect.SetTextStatusConnect("");                               // Очищаем строку панели статуса подключения по SSH

            Connect.Show(); // Открываем форму "Connect"
            this.Hide();    // Скрываем текущую форму
        }

        private void Form2_Load(object sender, EventArgs e)
        {
            this.FormBorderStyle = FormBorderStyle.FixedSingle;                 // Задаём фиксированный размер окна, без возможности менять его размер мышкой
            this.FormClosing += new FormClosingEventHandler(Form2_FormClosing); // Новый экземпляр делегата для обработки закрытия формы при нажатии на крестик в форме

            SetGroupNames(Connect.config);  // Устанавливаем из конфига текстовые названия (групп и подгрупп) элементов при загрузке формы и Вкл./Выкл. их

            //Регистрация событий изменения состояния радио-кнопок групп (для очистки имён файлов с панели "panelFiles")
            ServGroup1.CheckedChanged += new EventHandler(RadioGroup_CheckedChanged);
            ServGroup2.CheckedChanged += new EventHandler(RadioGroup_CheckedChanged);
            ServGroup3.CheckedChanged += new EventHandler(RadioGroup_CheckedChanged);
            ServGroup4.CheckedChanged += new EventHandler(RadioGroup_CheckedChanged);

            //Регистрация событий изменения состояния радио-кнопок подгрупп (для очистки имён файлов с панели "panelFiles")
            ServSubGroup1.CheckedChanged += new EventHandler(RadioSubGroup_CheckedChanged);
            ServSubGroup2.CheckedChanged += new EventHandler(RadioSubGroup_CheckedChanged);
            ServSubGroup3.CheckedChanged += new EventHandler(RadioSubGroup_CheckedChanged);
            ServSubGroup4.CheckedChanged += new EventHandler(RadioSubGroup_CheckedChanged);
            ServSubGroup5.CheckedChanged += new EventHandler(RadioSubGroup_CheckedChanged);
            ServSubGroup6.CheckedChanged += new EventHandler(RadioSubGroup_CheckedChanged);
            ServSubGroup7.CheckedChanged += new EventHandler(RadioSubGroup_CheckedChanged);
        }

        //Обработка события при нажатии закрытия окна "Form2"
        private void Form2_FormClosing(object sender, FormClosingEventArgs e)
        {
            DialogResult result = MessageBox.Show("Завершить работу с Admin DAVrun?", "Подтверждение", MessageBoxButtons.YesNo);
            if (result == DialogResult.Yes)
            {
                //Если подтвердили, то сначала отключаемся от SFTP
                if (sftpClient.IsConnected)
                {
                    sftpClient.Disconnect();
                }

                //Затем отключаемся от SSH
                if (sshClient.IsConnected)
                {
                    sshClient.Disconnect();
                }
            }
            else
            {
                e.Cancel = true;    // Иначе ничего не делаем
            }
        }

        //Ссылка на страницу проекта автора
        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            System.Diagnostics.Process.Start("https://github.com/Otto17/DAVrun");
        }

        //Вызов формы "About" (О программе)
        private void оПрограммеToolStripMenuItem_Click(object sender, EventArgs e)
        {
            About About_O_Programme = new();
            About_O_Programme.StartPosition = FormStartPosition.Manual;         // Устанавливаем начальную позицию формы
            About_O_Programme.Location = new System.Drawing.Point(              // Устанавливаем координаты расположения формы относительно текущей формы
                this.Location.X + (this.Width - About_O_Programme.Width) / 2,   // Вычисляем ширину относительно формы "About"
                this.Location.Y + (this.Height - About_O_Programme.Height) / 2  // Вычисляем высоту относительно формы "About"
            );
            About_O_Programme.ShowDialog(); // Показываем форму по центру окна формы "Management"
        }

        //Метод для вычисления SHA256 хеш-суммы установочного файла
        private string ComputeFileHash(string filePath)
        {
            using (SHA256 sha256 = SHA256.Create()) // Создаём объект для вычисления хеш-суммы
            {
                using (FileStream fileStream = File.OpenRead(filePath)) // Открываем файл по указанному пути
                {
                    byte[] hashBytes = sha256.ComputeHash(fileStream);  // Вычисляем хеш-сумму файла
                    StringBuilder hashString = new();     // Создаём объект "StringBuilder" для построения строки хеша

                    foreach (byte b in hashBytes)   // Проходим циклом по массиву и строим строку в hex формате
                    {
                        hashString.Append(b.ToString("x2"));
                    }
                    return hashString.ToString();   // Возвращаем строку с хеш-суммой
                }
            }
        }

        //Вывод уведомления при клике мышкой на строку с хеш
        private void labelhash_Click(object sender, EventArgs e)
        {
            if (!string.IsNullOrEmpty(labelhash.Text))  // Проверяем, не пустой ли текст в "labelhash"
            {
                Clipboard.SetText(labelhash.Text);                          // Копируем содержимое "labelhash" в буфер обмена
                ShowNotification("Хеш (SHA256) скопирован в буфер обмена"); // Показываем PUSH сообщение
            }
        }

        //Метод для показа уведомления
        private void ShowNotification(string message)
        {
            ToolTip toolTip = new();
            toolTip.Show(message, labelhash, 0, -20, 2500); // Уведомление будет показано 2.5 секунды
        }

        //Кнопка выбора setup файла
        private void btnSelectFile_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new())                   // Создаём объект класса "OpenFileDialog"
            {
                if (openFileDialog.ShowDialog() == DialogResult.OK)         // Проверяем, была ли нажата кнопка "Открыть"
                {
                    selectedFilePath = openFileDialog.FileName;             // Путь к выбранному файлу присваиваем переменной "selectedFilePath"
                    string fileHash = ComputeFileHash(selectedFilePath);    // Вычисляем хеш-сумму файла из этого пути
                    SelectedFile.Text = Path.GetFileName(selectedFilePath); // Показываем имя выбранного файла в label элементе
                    labelhash.Text = fileHash;                              // Показываем в другом label хеш-сумму выбранного файла

                    selectedFileHash = fileHash;    // Сохраняем хеш для использования при загрузке файла
                }
            }
        }

        //Кнопка загрузки setup файла
        private async void btnUploadFile_Click(object sender, EventArgs e)
        {
             if (string.IsNullOrEmpty(selectedFilePath) || string.IsNullOrEmpty(selectedFileHash))  // Проверка, что файл выбран
             {
                MessageBox.Show("Сначала выберите файл!", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
             }

            try
            {
                Config config = Connect.config;                                                 // Получаем значения из конфиг файла с формы "Connect"
                string fileExtension = Path.GetExtension(selectedFilePath);                     // Получаем расширение исходного файла
                string remoteFileName = "setup" + fileExtension + "_" + selectedFileHash;       // Формируем имя файла "setup.exe_хеш" с добавлением хеш-суммы после расширения
                string remoteDirectoryPath = $"{config.LinkCommonPath}/{config.LinkSetup}/";    // Полный путь на Linux сервере и название каталога, куда загрузится установочный файл

                CreateDirectoryIfNotExists(remoteDirectoryPath);    // Создаем каталог, если он ещё не существует

                if (!DeleteAllFilesInDirectory(remoteDirectoryPath))    // Удаляем все файлы в каталоге (удаляется старый установщик, независимо от названия)
                {
                    MessageBox.Show("Не удалось удалить старый установочный файл в каталоге. Очистите каталог вручную.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                string remoteFilePath = remoteDirectoryPath + remoteFileName;   // Путь к файлу в удалённом каталоге

                await Task.Run(() => UploadFileWithProgress(selectedFilePath, remoteFilePath)); // Загружаем установочный файл с указанно локального пути на удалённой сервер с отображением прогресс-бара
                MessageBox.Show("Файл успешно загружен!", "Успех", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Произошла ошибка: " + ex.Message, "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        //Вывод информации (размер и проценты) о загрузке
        private void UpdateProgressBar(double progress, long bytesReceived)
        {
            ProgressBar.Invoke((Action)(() =>   // Выполняем делегат синхронно в потоке с прогресс-баром
            {
                ProgressBar.Value = (int)progress;  // Конвертируем в int
                InfoProcess.Text = $"Загружено: {(int)progress}% ({((double)bytesReceived / 1048576):F1} МБ)"; // Отображаем на форме прогресс вида - "Загружено 27% (17,4 МБ)"
            }));
        }

        //Прогресс бар с информацией
        private void UploadFileWithProgress(string localFilePath, string remoteFilePath)
        {
            using (var fileStream = new FileStream(localFilePath, FileMode.Open))   // Открываем файловый поток для чтения файла по указанному пути
            {
                var fileSize = fileStream.Length;   // Получаем размер файла
                var buffer = new byte[8192];        // Создаём буфер размером 8192 байта
                int bytesRead;                      // Тут будем хранить кол-во считанных байт из файла
                long totalBytesRead = 0;            // Тут будет хранить общее кол-во байт считанных байт из файла

                using (var uploadStream = sftpClient.OpenWrite(remoteFilePath))         // Открываем поток для записи на удалённом сервере через SFTP
                {
                    while ((bytesRead = fileStream.Read(buffer, 0, buffer.Length)) > 0) // Крутимся в цикле, пока кол-во байт считанные из "fileStream" и записанные в "buffer", больше нуля
                    {
                        uploadStream.Write(buffer, 0, bytesRead);                   // Данные из "buffer" записываются в "uploadStream"
                        totalBytesRead += bytesRead;                                // "totalBytesRead" увеличивается на кол-во считанных байт "bytesRead"
                        double progress = (double)totalBytesRead / fileSize * 100;  // Вычисляем прогресс загрузки в процентном соотношении
                        UpdateProgressBar(progress, totalBytesRead);                // Обновляем индикатор хода с передачей в неё текущего процента прогресса и общего кол-ва считанных байт
                    }
                }
            }
        }

        //Функция удаления всех файлов в директории
        private bool DeleteAllFilesInDirectory(string remoteDirectoryPath)
        {
            try
            {
                var files = sftpClient.ListDirectory(remoteDirectoryPath)   // Получаем через SFTP список файлов с удалённого сервера
                    .Where(f => !f.IsDirectory && !f.IsSymbolicLink);       // Фильтруем список, оставляя только файлы, отбрасывая символические ссылки и директории

                foreach (var file in files) // Перебираем отфильтрованные элементы в переменной "files"
                {
                    sftpClient.DeleteFile(file.FullName);   // Удаляем файл из тех, что перебрали в цикле
                }

                return true;    // Возвращаем "true", если операция успешна
            }
            catch
            {
                return false;   // Иначе возвращаем "false"
            }
        }

        //Индивидуальная группа #4 (на вкладке "Файлы на сервере")
        private void ServGroup4_CheckedChanged(object sender, EventArgs e)
        {
            if (ServGroup4.Checked) // Если на "ServGroup4" отмечен
            {
                foreach (Control control in ServPanelFullGroup.Controls)    // Начинаем перебор всех элементов в контейнере "ServPanelFullGroup"
                {
                    if (control is RadioButton button && control.Name != "ServGroup4") // Если текущий элемент является "RadioButton" и его имя не равно "ServGroup4"
                    {
                        RadioButton radioButton = button;   // Присваиваем значение текущего элемента к "radioButton"
                        radioButton.Checked = false;        // Убираем точку с элемента
                        radioButton.Enabled = false;        // И выключаем его
                    }
                }
            }
            else // Иначе
            {
                foreach (Control control in ServPanelFullGroup.Controls)    // Начинаем перебор всех элементов в контейнере "ServPanelFullGroup"
                {
                    if (control is RadioButton button) // Если текущий элемент является "RadioButton"
                    {
                        if (control.Name == "ServSubGroup1")    // Если имя элемента равно "ServSubGroup1"
                        {
                            ServSubGroup1.Checked = true;   // Выбираем самую первую подгруппу
                        }
                        button.Enabled = GetSubGroupEnabledState(control.Name, Connect.config); // Получаем состояние доступности подгруппы для текущего элемента управления и устанавливаем свойство "Enabled" для текущего элемента
                    }
                }
            }
        }

        //Индивидуальная группа #4 (на вкладке "Загрузка файлов")
        private void LoadGroup4_CheckedChanged(object sender, EventArgs e)
        {
            if (LoadGroup4.Checked) // Если на "LoadGroup4" отмечен
            {
                foreach (Control control in LoadPanelFullGroup.Controls)    // Начинаем перебор всех элементов в контейнере "LoadPanelFullGroup"
                {
                    if (control is RadioButton button && control.Name != "LoadGroup4") // Если текущий элемент является "RadioButton" и его имя не равно "LoadGroup4"
                    {
                        RadioButton radioButton = button;   // Присваиваем значение текущего элемента к "radioButton"
                        radioButton.Checked = false;        // Убираем точку с элемента
                        radioButton.Enabled = false;        // И выключаем его
                    }
                }
            }
            else // Иначе
            {
                foreach (Control control in LoadPanelFullGroup.Controls)    // Начинаем перебор всех элементов в контейнере "LoadPanelFullGroup"
                {
                    if (control is RadioButton button) // Если текущий элемент является "RadioButton"
                    {
                        if (control.Name == "LoadSubGroup1")    // Если имя элемента равно "LoadSubGroup1"
                        {
                            LoadSubGroup1.Checked = true;   // Выбираем самую первую подгруппу
                        }
                        button.Enabled = GetSubGroupEnabledState(control.Name, Connect.config); // Получаем состояние доступности подгруппы для текущего элемента управления и устанавливаем свойство "Enabled" для текущего элемента
                    }
                }
            }
        }

        // Получение состояния подгруппы из конфигурации (требуется версия C# не ниже 9.0)
        private bool GetSubGroupEnabledState(string subGroupName, Config config)
        {
            return subGroupName switch // Проверяем значение "subGroupName"
            {
                "ServSubGroup2" or "LoadSubGroup2" => config.OnOffSubGroup2,
                "ServSubGroup3" or "LoadSubGroup3" => config.OnOffSubGroup3,
                "ServSubGroup4" or "LoadSubGroup4" => config.OnOffSubGroup4,
                "ServSubGroup5" or "LoadSubGroup5" => config.OnOffSubGroup5,
                "ServSubGroup6" or "LoadSubGroup6" => config.OnOffSubGroup6,
                "ServSubGroup7" or "LoadSubGroup7" => config.OnOffSubGroup7,
                _ => true   // Если "subGroupName" не соответствует ни одному из указанных случаев, возвращаем true
            };
        }

        //Кнопка "Загрузить имена ПК"
        private void btnDownlPCName_Click(object sender, EventArgs e)
        {
            if (draggedFiles.Count == 0)    // Если кол-во элементов равно нулю, выводим сообщение об ошибке
            {
                MessageBox.Show("Нет файлов для копирования.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (!sftpClient.IsConnected)    // Если клиент SFTP не подключен, выводим сообщение об ошибке
            {
                MessageBox.Show("SFTP-клиент не подключен.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string uploadPath = GetUploadPath();    // Получаем путь загрузки
            if (string.IsNullOrEmpty(uploadPath))   // Если результат равен пустой строке, выводим сообщение об ошибке
            {
                MessageBox.Show("Не выбрана группа или подгруппа для загрузки файлов.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            CreateDirectoryIfNotExists(uploadPath); // Проверка существования и создание пути, если его нет

            foreach (var file in draggedFiles)  // Перебираем все файлы в коллекции "draggedFiles"
            {
                using (var fileStream = new FileStream(file, FileMode.Open))    // открываем файловый поток для каждого файла
                {
                    sftpClient.UploadFile(fileStream, uploadPath + Path.GetFileName(file)); // Загружаем файлы на сервер
                }
            }

            MessageBox.Show("Все файлы успешно скопированы.", "Информация", MessageBoxButtons.OK, MessageBoxIcon.Information);

            draggedFiles.Clear();   // Очищаем список после загрузки
            labelDrag.Text = "Перетащите файлы сюда\r\nили кликните в эту область\r\n";
        }

        //Функция создания пути на сервере, если его не существует
        private void CreateDirectoryIfNotExists(string path)
        {
            //Строка "path" разбивается на массив "directories" по символу слэша '/' с удалением пустых элементов
            string[] directories = path.Split(new char[] { '/' }, StringSplitOptions.RemoveEmptyEntries);
            string currentPath = string.Empty;  // Инициализируем переменную пустой строкой

            foreach (var dir in directories)    // Запускаем цикл по массиву "directories"
            {
                currentPath += "/" + dir;   // К текущему пути добавляем '/' и текущую директорию "dir"

                if (!sftpClient.Exists(currentPath))    // Проверяем существование директории
                {
                    sftpClient.CreateDirectory(currentPath);    // Если директория не существует, создаём её
                }
            }
        }

        //Метод вызывается при перетаскивании файла на панель, без отпускания клика мыши (Drag and Drop)
        private void panel4_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))    // Проверяем, присутствует ли в перетаскиваемых данных формат файлов
            {
                string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);    // Если есть формат файлов, то извлекаем список файлов из данных перетаскивания
                if (files.Any(file => Directory.Exists(file)))                      // Если среди файлов есть папки, то запрещаем перетаскивание
                {
                    e.Effect = DragDropEffects.None;                    // Запрещаем перетаскивание, если среди файлов есть папки
                    labelDrag.Text = "Перетаскивание папок запрещено";  // Изменяем текст на панели
                }
                else
                {
                    e.Effect = DragDropEffects.Copy;    // Устанавливаем эффект перетаскивания на "Copy"
                    labelDrag.Text = "Отпустите мышь";  // Изменяем текст на панели
                }
            }
            else
            {
                e.Effect = DragDropEffects.None;    // Если формат файлов отсутствует, то запрещаем перетаскивание файлов
            }
        }

        //Метод, выполняющийся при перетаскивании указателя мыши за пределы панели (Drag and Drop)
        private void panel4_DragLeave(object sender, EventArgs e)
        {
            if (draggedFiles.Count > 0) // Если кол-во в "draggedFiles" файлов больше нуля
            {
                labelDrag.Text = "Файлы добавлены: " + draggedFiles.Count;  // Изменяем текст на панели и добавляем в него кол-во файлов
            }
            else
            {
                labelDrag.Text = "Перетащите файлы сюда\r\nили кликните в эту область\r\n";
            }
        }

        //Метод вызывается при событии перетаскивании файлов на панель и отпускании клика мышки (Drag and Drop)
        private void panel4_DragDrop(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop))    // Проверяем, присутствует ли в перетаскиваемых данных формат файлов
            {
                string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);    // Если есть формат файлов, то извлекаем список файлов из данных перетаскивания
                if (files.Any(file => Directory.Exists(file)))                      // Если среди файлов есть папки, то запрещаем перетаскивание
                {
                    labelDrag.Text = "Перетаскивание папок запрещено";  // Изменяем текст на панели
                }
                else
                {
                    draggedFiles.Clear();                                   // Очистка предыдущих файлов
                    draggedFiles.AddRange(files);                           // Добавляем файлы из массива "files" в список "draggedFiles"
                    labelDrag.Text = "Файлы добавлены: " + files.Length;    // Изменяем текст на панели и добавляем в него кол-во файлов
                }
            }
        }

        //Метод вызывается при клике мышкой на панель, вместо перетаскивания файлов (Drag and Drop)
        private void panel4_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new())    // Создаём экземпляр класса
            {
                openFileDialog.Multiselect = true;                              // Позволяем выбирать несколько файлов
                if (openFileDialog.ShowDialog() == DialogResult.OK)             // Проверяем, была ли нажата кнопка "Открыть"
                {
                    draggedFiles.Clear();                                       // Очистка предыдущих файлов
                    draggedFiles.AddRange(openFileDialog.FileNames);            // Добавляем в список "draggedFiles" имена выбранных файлов
                    labelDrag.Text = "Файлы добавлены: " + draggedFiles.Count;  // Изменяем текст на панели и добавляем в него кол-во файлов
                }
            }
        }

        //Рисуем красивую пунктирную линию для Drag and Drop панели
        private void panel4_Paint(object sender, PaintEventArgs e)
        {
            Pen pen = new Pen(Color.WhiteSmoke, 2); // Создаём объект "Pen" с цветом "WhiteSmoke" и толщиной "2"
            pen.DashPattern = new float[] { 2, 2 }; // Устанавливаем пунктирный стиль для пера, который будет последовательно рисовать 2 пикселя и пропускать 2 пикселя
            
            e.Graphics.DrawRectangle(pen, 1, 1, panelDrop.Width - 2, panelDrop.Height - 2); // Рисуем прямоугольник внутри панели с отступами от ширины и высоты самой панели
        }

        //Функция для работы с путями на сервере при выборе группы и подгруппы
        private string GetUploadPath()
        {
            Config config = Connect.config; // Получаем значения из конфиг файла с формы "Connect"

            if (LoadGroup4.Checked && ServGroup4.Checked)   // Если выбраны четвёртые группы (выбираются всегда синхронно)
            {
                return $"{config.LinkCommonPath}/Name_PC/{config.LinkGroup4}/"; // Возвращаем {полный путь до каталогов с началом имён}/{название группы} - для индивидуальной группы #4
            }

            //Создаём пустые строки для формирования пути группы и подгруппы
            string group = string.Empty;
            string subgroup = string.Empty;

            if (LoadGroup1.Checked && ServGroup1.Checked)   // Если выбрана первая группа (выбираются всегда синхронно)
            {
                group = config.LinkGroup1; // Подставляем ссылку из конфига в группу
            }
            else if (LoadGroup2.Checked && ServGroup2.Checked)  // Далее аналогично
            {
                group = config.LinkGroup2;
            }
            else if (LoadGroup3.Checked && ServGroup3.Checked)
            {
                group = config.LinkGroup3;
            }

            if (LoadSubGroup1.Checked && ServSubGroup1.Checked) // Если выбрана первая подгруппа (выбираются всегда синхронно)
            {
                subgroup = config.LinkSubGroup1;   // Подставляем ссылку из конфига в подгруппу
            }
            else if (LoadSubGroup2.Checked && ServSubGroup2.Checked)    // Далее аналогично
            {
                subgroup = config.LinkSubGroup2;
            }
            else if (LoadSubGroup3.Checked && ServSubGroup3.Checked)
            {
                subgroup = config.LinkSubGroup3;
            }
            else if (LoadSubGroup4.Checked && ServSubGroup4.Checked)
            {
                subgroup = config.LinkSubGroup4;
            }
            else if (LoadSubGroup5.Checked && ServSubGroup5.Checked)
            {
                subgroup = config.LinkSubGroup5;
            }
            else if (LoadSubGroup6.Checked && ServSubGroup6.Checked)
            {
                subgroup = config.LinkSubGroup6;
            }
            else if (LoadSubGroup7.Checked && ServSubGroup7.Checked)
            {
                subgroup = config.LinkSubGroup7;
            }

            if (!string.IsNullOrEmpty(group) && !string.IsNullOrEmpty(subgroup))    // Если строки группы и подгруппы не пустые
            {
                return $"{config.LinkCommonPath}/Name_PC/{group}/{subgroup}/";  // Возвращаем {полный путь до каталогов с началом имён}/Name_PC/{название группы}/{название подгруппы}
            }

            return string.Empty;    // В противном случае возвращаем пустую строку
        }

        //Очищаем панель с файлами "panelFiles" при выборе любой группы
        private void RadioGroup_CheckedChanged(object sender, EventArgs e)
        {
            if ((sender as RadioButton).Checked)    //  Если текущая радио-кнопка группы, которая вызвала событие, отмечена
            {
                ClearFileList();    // Очищаем список файлов на панели
            }
        }

        //Очищаем панель с файлами "panelFiles" при выборе любой подгруппы
        private void RadioSubGroup_CheckedChanged(object sender, EventArgs e)
        {
            if ((sender as RadioButton).Checked)    //  Если текущая радио-кнопка подгруппы, которая вызвала событие, отмечена
            {
                ClearFileList();    // Очищаем список файлов на панели
            }
        }

        //Функция очистки списка файлов на панели
        private void ClearFileList()
        {
            panelFiles.Controls.Clear();    // Очищаем от названий файлов и кнопок внутри панели "panelFiles" (на вкладке "Файлы на сервере")
            panelInstall.Controls.Clear();  // Очищаем от названий файлов и кнопок внутри панели "panelInstall" (на вкладке "Процесс установки")
            fileCheckBoxes.Clear();         // Очищаем чекбоксы с панелей "Файлы на сервере" и "Процесс установки"
        }

        //Кнопка "Показать файлы" на вкладке "Файлы на сервере"
        private void btnServShow_Click(object sender, EventArgs e)
        {
            string path = GetUploadPath();  // Получаем путь

            if (string.IsNullOrEmpty(path)) // Если путь пустой или null
            {
                MessageBox.Show("Не выбрана группа или подгруппа для отображения файлов.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try
            {
                ClearFileList();    // Очищаем список файлов на панели

                //Создаем панель для файлов, которая будет внутри основной панели
                Panel filePanel = new()
                {
                    Location = new Point(10, 45),                                   // Расположение нового окна в панели по координатам
                    Size = new Size(panelFiles.Width - 20, panelFiles.Height - 60), // Высота и ширина нового окна в панели равна высоте и ширине "panelFiles" за вычетом указанных пикселей
                    BorderStyle = BorderStyle.FixedSingle,                          // Устанавливаем стиль рамки
                    HorizontalScroll = { Enabled = false, Visible = false },        // Отключаем горизонтальную прокрутку
                    AutoScroll = true                                               // Включаем авто-прокрутку для вертикальной полосы
                };

                //Сортировка файлов в алфавитном порядке
                var files = sftpClient.ListDirectory(path)              // Получаем список файлов с директории по указанному пути
                    .Where(f => !f.IsDirectory && !f.IsSymbolicLink)    // Фильтруем список, исключая каталоги и символические ссылки
                    .OrderBy(f => f.Name);                              // Сортируем файлы по имени

                //Формируем список файлов внутри панели
                int yOffset = 10;   // Начальное расположение файла на панели

                foreach (var file in files) // Перебор файлов в цикле
                {
                    CheckBox fileCheckBox = new()   // Для каждого файла создаём новый чекбокс
                    {
                        Text = file.Name,                               // Устанавливаем название файла
                        Location = new Point(5, yOffset),               // Расположение на панели
                        Width = filePanel.Width - 25,                   // Уменьшение ширины и добавление отступа
                        Anchor = AnchorStyles.Top | AnchorStyles.Left   // Фиксированное выравнивание
                    };
                    yOffset += 25;  // Увеличиваем расположение для нового файла (на подобии абзаца)

                    fileCheckBoxes.Add(fileCheckBox);       // Созданный чекбокс добавляется в список "fileCheckBoxes"
                    filePanel.Controls.Add(fileCheckBox);   // Созданный чекбокс добавляется на панель
                }

                //Добавление кнопок внутри панели "Выбрать всё" и "Снять выделение"
                Button btnSelectAll = new() // Создаём объект типа кнопка
                {
                    Text = "Выделить всё",              // Даём ей название
                    Location = new Point(10, 10),       // Расположение кнопки
                    Width = panelFiles.Width / 2 - 15   // Ширина кнопки равна половине ширины "panelFiles" минус 15 пикселей
                };

                btnSelectAll.Click += (s, args) => SetCheckState(true); // Вызываем метод при клике на кнопку

                Button btnDeselectAll = new()   // Создаём обект типа кнопка
                {
                    Text = "Снять выделение",                           // Даём ей название
                    Location = new Point(panelFiles.Width / 2 + 5, 10), // Расположение кнопки (половина ширины панели "panelFiles" плюс 5 пикселей)
                    Width = panelFiles.Width / 2 - 15                   // Ширина кнопки равна половине ширины "panelFiles" минус 15 пикселей
                };

                btnDeselectAll.Click += (s, args) => SetCheckState(false);  // Вызываем метод при клике на кнопку

                panelFiles.Controls.Add(btnSelectAll);      // Добавляем на панель кнопку "Выделить всё"
                panelFiles.Controls.Add(btnDeselectAll);    // Добавляем на панель кнопку "Снять выделение"
                panelFiles.Controls.Add(filePanel);         // Добавляем на панель "panelFiles" виртуальную панель "filePanel"

                if (!files.Any())   // Если не нашли файлов по пути, выводим сообщение
                {
                    MessageBox.Show("Файлы не найдены в выбранной директории.", "Информация", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Ошибка при получении списка файлов: {ex.Message}", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        //Функция установки или снятия галочек в панелях
        private void SetCheckState(bool state)
        {
            foreach (var checkBox in fileCheckBoxes)    // Крутим цикл по всем элементам "fileCheckBoxes"
            {
                checkBox.Checked = state;   // Устанавливаем или снимаем галочку для каждого элемента (в зависимости, что передали в функцию)
            }
        }
        
        //Кнопка "Установить на ПК" на вкладке "Файлы на Сервере"
        private void btnServInstall_Click(object sender, EventArgs e)
        {
            Config config = Connect.config;                                         // Получаем значения из конфиг файла с формы "Connect"
            string sourcePath = GetUploadPath();                                    // Получаем исходный путь
            string targetPath = $"{config.LinkCommonPath}/{config.LinkInstall}/";   // Получаем {полный путь до каталогов с началом имён}/{название каталога на сервре}, куда будем копировать выбранные файлы
            CreateDirectoryIfNotExists(targetPath);                                 // Создаём путь на сервере, если он ещё не существует


            var checkedFiles = fileCheckBoxes.Where(c => c.Checked).ToList();   // Создаём список элементов "checkedFiles", отмеченные из списка "fileCheckBoxes"
           
            if (!checkedFiles.Any())    // Если список пуст, выводим сообщение
            {
                MessageBox.Show("Нет выбранных файлов для установки.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            //Выводим окно с подтверждением
            DialogResult result = MessageBox.Show("Точно устанавливаем выбранные файлы?", "Подтверждение установки", MessageBoxButtons.YesNo);
            if (result != DialogResult.Yes)
            {
                return;
            }

            try
            {
                foreach (var checkBox in checkedFiles)  // Цикл поочерёдной загрузки файлов
                {
                    string sourceFilePath = sourcePath + checkBox.Text; // Путь к исходному файлу
                    string targetFilePath = targetPath + checkBox.Text; // Путь к целевому файлу

                    using (var fileStream = sftpClient.OpenRead(sourceFilePath))    // Открытие файла для чтения через SFTP
                    {
                        sftpClient.UploadFile(fileStream, targetFilePath, true);    // Загружаем файл в целевую директорию через SFTP
                    }
                }
                MessageBox.Show("Файлы отправлены на установку.", "Информация", MessageBoxButtons.OK, MessageBoxIcon.Information);

                foreach (var checkBox in fileCheckBoxes)    // Цикл для снятия галочек с каждого чекбокса
                {
                    checkBox.Checked = false;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Ошибка при отправке файлов на установку: {ex.Message}", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        //Кнопка "Удалить файлы" на вкладке "Файлы на Сервере"
        private void btnServDel_Click(object sender, EventArgs e)
        {
            var checkedFiles = fileCheckBoxes.Where(c => c.Checked).ToList();   // Создаём список "checkedFiles", в который добавляются все элементы "fileCheckBoxes" с true
            
            if (!checkedFiles.Any())    // Проверка, есть ли элементы в списке "checkedFiles"
            {
                MessageBox.Show("Нет выбранных файлов для удаления.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            //Выводим окно с подтверждением
            DialogResult result = MessageBox.Show("Вы уверены, что хотите удалить выбранные файлы?", "Подтверждение удаления", MessageBoxButtons.YesNo);
            if (result != DialogResult.Yes)
            {
                return;
            }

            try
            {
                foreach (var checkBox in checkedFiles)  // Формируем путь к файлам через цикл
                {
                    string filePath = GetUploadPath() + checkBox.Text;  // Путь к файлу с отмеченным именем
                    sftpClient.DeleteFile(filePath);                    // Удаляем файл через SFTP
                }
                MessageBox.Show("Файлы успешно удалены.", "Информация", MessageBoxButtons.OK, MessageBoxIcon.Information);

                btnServShow_Click(sender, e); // Обновляем список с помощью функции
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Ошибка при удалении файлов: {ex.Message}", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        //Синхронно выбирает элементы "radio" на вкладках "Файлы на сервере" и "Загрузка файлов", что бы не было каши, что загрузили в одно место, а показывает файлы с какой то другой группы....
        //Для групп "Файлы на сервере"
        private void ServGroup1_Click(object sender, EventArgs e)
        {
            LoadGroup1.Checked = true;   // Если нажата на вкладке "Файлы на сервере" LoadGroup1, тогда тоже так же выбираем LoadGroup1, только на вкладке "Загрузка файлов". Аналогично и для других пунктов
        }

        private void ServGroup2_Click(object sender, EventArgs e)
        {
            LoadGroup2.Checked = true;
        }

        private void ServGroup3_Click(object sender, EventArgs e)
        {
            LoadGroup3.Checked = true;
        }

        private void ServGroup4_Click(object sender, EventArgs e)
        {
            LoadGroup4.Checked = true;
        }
        //Для групп "Загрузка файлов"
        private void LoadGroup1_Click(object sender, EventArgs e)
        {
            ServGroup1.Checked = true;   // Если нажата на вкладке "Загрузка файлов" ServGroup1, тогда тоже нажимаем ServGroup1, только на вкладке "Файлы на сервере". Аналогично и для других пунктов
        }

        private void LoadGroup2_Click(object sender, EventArgs e)
        {
            ServGroup2.Checked = true;
        }

        private void LoadGroup3_Click(object sender, EventArgs e)
        {
            ServGroup3.Checked = true;
        }

        private void LoadGroup4_Click(object sender, EventArgs e)
        {
            ServGroup4.Checked = true;
        }

        //Для подгрупп "Файлы на сервере"
        private void ServSubGroup1_Click(object sender, EventArgs e)
        {
            LoadSubGroup1.Checked = true;  // Если нажата на вкладке "Файлы на сервере" LoadSubGroup1, тогда тоже нажимаем LoadSubGroup1, только на вкладке "Загрузка файлов". Аналогично и для других пунктов
        }

        private void ServSubGroup2_Click(object sender, EventArgs e)
        {
            LoadSubGroup2.Checked = true;
        }

        private void ServSubGroup3_Click(object sender, EventArgs e)
        {
            LoadSubGroup3.Checked = true;
        }

        private void ServSubGroup4_Click(object sender, EventArgs e)
        {
            LoadSubGroup4.Checked = true;
        }

        private void ServSubGroup5_Click(object sender, EventArgs e)
        {
            LoadSubGroup5.Checked = true;
        }

        private void ServSubGroup6_Click(object sender, EventArgs e)
        {
            LoadSubGroup6.Checked = true;
        }

        private void ServSubGroup7_Click(object sender, EventArgs e)
        {
            LoadSubGroup7.Checked = true;
        }

        //Для подгрупп "Загрузка файлов"
        private void LoadSubGroup1_Click(object sender, EventArgs e)
        {
            ServSubGroup1.Checked = true;  // Если нажата на вкладке "Загрузка файлов" ServSubGroup1, тогда тоже нажимаем ServSubGroup1, только на вкладке "Файлы на сервере". Аналогично и для других пунктов
        }

        private void LoadSubGroup2_Click(object sender, EventArgs e)
        {
            ServSubGroup2.Checked = true;
        }

        private void LoadSubGroup3_Click(object sender, EventArgs e)
        {
            ServSubGroup3.Checked = true;
        }

        private void LoadSubGroup4_Click(object sender, EventArgs e)
        {
            ServSubGroup4.Checked = true;
        }

        private void LoadSubGroup5_Click(object sender, EventArgs e)
        {
            ServSubGroup5.Checked = true;
        }

        private void LoadSubGroup6_Click(object sender, EventArgs e)
        {
            ServSubGroup6.Checked = true;
        }

        private void LoadSubGroup7_Click(object sender, EventArgs e)
        {
            ServSubGroup7.Checked = true;
        }

        //Установка на форме названий для групп и подгрупп из конфига
        internal void SetGroupNames(Config config)
        {
            //Для вкладки "Загрузка файлов"
            LoadGroup1.Text = config.Group1;
            LoadGroup2.Text = config.Group2;
            LoadGroup3.Text = config.Group3;
            LoadGroup4.Text = config.Group4;

            LoadSubGroup1.Text = config.SubGroup1;
            LoadSubGroup2.Text = config.SubGroup2;
            LoadSubGroup3.Text = config.SubGroup3;
            LoadSubGroup4.Text = config.SubGroup4;
            LoadSubGroup5.Text = config.SubGroup5;
            LoadSubGroup6.Text = config.SubGroup6;
            LoadSubGroup7.Text = config.SubGroup7;

            //Для вкладки "Файлы на сервере"
            ServGroup1.Text = config.Group1;
            ServGroup2.Text = config.Group2;
            ServGroup3.Text = config.Group3;
            ServGroup4.Text = config.Group4;

            ServSubGroup1.Text = config.SubGroup1;
            ServSubGroup2.Text = config.SubGroup2;
            ServSubGroup3.Text = config.SubGroup3;
            ServSubGroup4.Text = config.SubGroup4;
            ServSubGroup5.Text = config.SubGroup5;
            ServSubGroup6.Text = config.SubGroup6;
            ServSubGroup7.Text = config.SubGroup7;

            //Чекбоксы (Вкл./Выкл.)
            ServGroup2.Enabled = LoadGroup2.Enabled = config.OnOffGroup2;
            ServGroup3.Enabled = LoadGroup3.Enabled = config.OnOffGroup3;
            ServGroup4.Enabled = LoadGroup4.Enabled = config.OnOffGroup4;

            ServSubGroup2.Enabled = LoadSubGroup2.Enabled = config.OnOffSubGroup2;
            ServSubGroup3.Enabled = LoadSubGroup3.Enabled = config.OnOffSubGroup3;
            ServSubGroup4.Enabled = LoadSubGroup4.Enabled = config.OnOffSubGroup4;
            ServSubGroup5.Enabled = LoadSubGroup5.Enabled = config.OnOffSubGroup5;
            ServSubGroup6.Enabled = LoadSubGroup6.Enabled = config.OnOffSubGroup6;
            ServSubGroup7.Enabled = LoadSubGroup7.Enabled = config.OnOffSubGroup7;
        }

        //Кнопка "Обновить" на вкладке "Процесс установки"
        private void btnInstallRefresh_Click(object sender, EventArgs e)
        {
            Config config = Connect.config;                                 // Получаем значения из конфиг файла с формы "Connect"
            string path = $"{config.LinkCommonPath}/{config.LinkInstall}/"; // Получаем {полный путь до каталогов с началом имён}/{название каталога на сервере}, где будет просматривать список файлов

            try
            {
                ClearFileList();    // Очищаем список файлов на панели

                //Создаем панель для файлов, которая будет внутри основной панели
                Panel filePanelInstall = new()
                {
                    Location = new Point(10, 45),                                       // Расположение нового окна в панели по координатам
                    Size = new Size(panelInstall.Width - 20, panelInstall.Height - 60), // Высота и ширина нового окна в панели равна высоте и ширине "panelInstall" за вычетом указанных пикселей
                    BorderStyle = BorderStyle.FixedSingle,                              // Устанавливаем стиль рамки
                    HorizontalScroll = { Enabled = false, Visible = false },            // Отключаем горизонтальную прокрутку
                    AutoScroll = true                                                   // Включаем авто-прокрутку для вертикальной полосы
                };

                //Сортировка файлов в алфавитном порядке
                var files = sftpClient.ListDirectory(path)              // Получаем список файлов с директории по указанному пути
                    .Where(f => !f.IsDirectory && !f.IsSymbolicLink)    // Фильтруем список, исключая каталоги и символические ссылки
                    .OrderBy(f => f.Name);                              // Сортируем файлы по имени

                //Формируем список файлов внутри панели
                int yOffset = 10;   // Начальное расположение файла на панели

                foreach (var file in files) // Перебор файлов в цикле
                {
                    CheckBox fileCheckBox = new()   // Для каждого файла создаём новый чекбокс
                    {
                        Text = file.Name,                               // Устанавливаем название файла
                        Location = new Point(5, yOffset),               // Расположение на панели
                        Width = filePanelInstall.Width - 25,            // Уменьшение ширины и добавление отступа
                        Anchor = AnchorStyles.Top | AnchorStyles.Left   // Фиксированное выравнивание
                    };
                    yOffset += 25;  // Увеличиваем расположение для нового файла (на подобии абзаца)

                    fileCheckBoxes.Add(fileCheckBox);               // Созданный чекбокс добавляется в список "fileCheckBoxes"
                    filePanelInstall.Controls.Add(fileCheckBox);    // Созданный чекбокс добавляется на панель
                }

                //Проверяем и удаляем предыдущие кнопки, если они есть
                var existingBtnSelectAll = panelInstall.Controls.OfType<Button>().FirstOrDefault(b => b.Text == "Выделить всё");
                var existingBtnDeselectAll = panelInstall.Controls.OfType<Button>().FirstOrDefault(b => b.Text == "Снять выделение");

                if (existingBtnSelectAll != null && existingBtnDeselectAll != null) // Если оба объекта существуют, то удаляем их
                {
                    panelInstall.Controls.Remove(existingBtnSelectAll);
                    panelInstall.Controls.Remove(existingBtnDeselectAll);
                }

                //Добавление кнопок внутри панели "Выбрать всё" и "Снять выделение"
                Button btnSelectAll = new() // Создаём объект типа кнопка
                {
                    Text = "Выделить всё",              // Даём ей название
                    Location = new Point(10, 10),       // Расположение кнопки
                    Width = panelInstall.Width / 2 - 15 // Ширина кнопки равна половине ширины "panelInstall" минус 15 пикселей
                };

                btnSelectAll.Click += (s, args) => SetCheckState(true); // Вызываем метод при клике на кнопку

                Button btnDeselectAll = new()   // Создаём объект типа кнопка
                {
                    Text = "Снять выделение",                               // Даём ей название
                    Location = new Point(panelInstall.Width / 2 + 5, 10),   // Расположение кнопки (половина ширины панели "panelInstall" плюс 5 пикселей)
                    Width = panelInstall.Width / 2 - 15                     // Ширина кнопки равна половине ширины "panelInstall" минус 15 пикселей
                };

                btnDeselectAll.Click += (s, args) => SetCheckState(false);  // Вызываем метод при клике на кнопку

                panelInstall.Controls.Add(btnSelectAll);        // Добавляем на панель кнопку "Выделить всё"
                panelInstall.Controls.Add(btnDeselectAll);      // Добавляем на панель кнопку "Снять выделение"
                panelInstall.Controls.Add(filePanelInstall);    // Добавляем на панель "panelInstall" виртуальную панель "filePanelInstall"

                if (!files.Any())   // Если не нашли файлов по пути
                {
                    CheckAutoUpdateSetup.Checked = false;   // Отключаем автообновление, так как список ожидания установок пуст
                    MessageBox.Show("В данный момент установок на ПК не производится.", "Информация", MessageBoxButtons.OK, MessageBoxIcon.Information);    // Выводим сообщение
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Ошибка при получении списка файлов: {ex.Message}", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        //Кнопка "Отменить установку" на вкладке "Процесс установки"
        private void btnStopInstall_Click(object sender, EventArgs e)
        {
            var filesToRemove = fileCheckBoxes.Where(cb => cb.Checked).ToList();    // Создаём список, в котором будут храниться все отмеченные чекбоксы

            if (!filesToRemove.Any())   // Если список пустой, выводим сообщение
            {
                MessageBox.Show("Нет выбранных компьютеров для удаления.", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            //Выводим окно с подтверждением
            DialogResult result = MessageBox.Show("Отменить установку на выбранных компьютерах?", "Отмена установки", MessageBoxButtons.YesNo);
            if (result != DialogResult.Yes)
            {
                return;
            }

            try
            {
                foreach (var fileCheckBox in filesToRemove) // Перебор файлов в цикле
                {
                    Config config = Connect.config;                                 // Получаем значения из конфиг файла с формы "Connect"
                    string path = $"{config.LinkCommonPath}/{config.LinkInstall}/"; // Получаем {полный путь до каталогов с началом имён}/{название каталога на сервре}, где будет удалять список файлов

                    string filePath = path + fileCheckBox.Text; // Формируем путь к файлу с отмеченным именем
                    sftpClient.DeleteFile(filePath);            // Удаляем файл через SFTP

                    panelInstall.Controls.Remove(fileCheckBox); // Удаляем чекбокс с панели "panelInstall"
                    fileCheckBoxes.Remove(fileCheckBox);        // Удаляем чекбокс с виртуальной панели "fileCheckBox"
                }

                MessageBox.Show("Установка на выбранных компьютеров отменена.", "Информация", MessageBoxButtons.OK, MessageBoxIcon.Information);

                btnInstallRefresh_Click(sender, e); // Обновляем список с помощью функции
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Ошибка при отмене: {ex.Message}", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }


        //Метод для инициализации автообновления
        private void InitializeAutoUpdateControls()
        {
            //Установка обработчиков событий
            CheckAutoUpdateSetup.CheckedChanged += (s, e) => ToggleAutoUpdate(CheckAutoUpdateSetup.Checked);    // При изменении значения "CheckAutoUpdateSetup", вызывается метод "ToggleAutoUpdate()"
            AutoUpdate_30s.CheckedChanged += (s, e) => UpdateTimerInterval();                                   // При изменении значения любого "AutoUpdate_*", вызывается метод "UpdateTimerInterval()"
            AutoUpdate_1m.CheckedChanged += (s, e) => UpdateTimerInterval();
            AutoUpdate_2m.CheckedChanged += (s, e) => UpdateTimerInterval();

            //Инициализация таймера
            autoUpdateTimer = new System.Timers.Timer();                                                                    // Создаём новый объект таймера
            autoUpdateTimer.Elapsed += (s, e) => this.Invoke((MethodInvoker)delegate { btnInstallRefresh_Click(s, e); });   // Обработчик события. "Elapsed" вызывает метод "Invoke" с делегатом "MethodInvoker", который в свою очередь вызывает метод "btnInstallRefresh_Click()"
            autoUpdateTimer.AutoReset = true;                                                                               // Авто-перезапуск таймера после каждого срабатывания

            ToggleRadioButtons(false);  // Изначально все элементы "AutoUpdate_*" отключены
        }


        //Функция запуска или остановки автообновления
        private void ToggleAutoUpdate(bool isEnabled)
        {
            ToggleRadioButtons(isEnabled);  // Активируем или деактивируем элементы "AutoUpdate_*"

            if (isEnabled)  // Если элементы "AutoUpdate_*" активны
            {
                UpdateTimerInterval();  // Обновляем интервал таймера
            }
            else
            {
                autoUpdateTimer.Stop(); // Останавливаем таймер
            }
        }


        //Функция установки и запуска интервала для таймера автообновления
        private void UpdateTimerInterval()
        {
            if (CheckAutoUpdateSetup.Checked)   // Если галочка на чекбоксе "Автоообновление", тогда устанавливаем таймер на то время, на котором стоит точка
            {
                if (AutoUpdate_30s.Checked) autoUpdateTimer.Interval = 30000;       // 30 секунд
                else if (AutoUpdate_1m.Checked) autoUpdateTimer.Interval = 60000;   // 1 минута
                else if (AutoUpdate_2m.Checked) autoUpdateTimer.Interval = 120000;  // 2 минуты

                autoUpdateTimer.Start();    // Запускаем таймер
            }
        }


        //Метод активирует или деактивирует элементы "AutoUpdate_*"
        private void ToggleRadioButtons(bool enabled)
        {
            AutoUpdate_30s.Enabled = enabled;
            AutoUpdate_1m.Enabled = enabled;
            AutoUpdate_2m.Enabled = enabled;
        }
    }
}