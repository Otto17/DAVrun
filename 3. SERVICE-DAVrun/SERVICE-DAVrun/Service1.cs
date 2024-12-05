using System;                           // Библиотека предоставляет доступ к базовым классам и функциональности .NET Framework
using System.IO;                        // Библиотека отвечает за ввод и вывод данных, включая чтение и запись файлов
using System.ServiceProcess;            // Библиотека для работаты со службами Windows
using System.Diagnostics;               // Библиотека для работы с процессами и системными ресурсами компьютера
using System.Runtime.InteropServices;   // Библиотека предоставляет средства для взаимодействия с кодом на нативном уровне
using System.Text;                      // Библиотека предоставляет классы и методы для работы с текстом, включая работу со строками, преобразованием кодировок и форматированием текста
using System.Timers;                    // Библиотека для работы с таймерами и выполнению действий по истечению указанного времени

namespace SERVICE_DAVrun
{
    public partial class DAVrunService : ServiceBase
    {
        //Настройки службы
        private const int START_TIMER = 600000;                                     // Время (в миллисекундах), через которое будет циклически запускаться программа (по умолчанию 10 минут)
        private const string PROGRAM_PATH = @"C:\Program Files\DAVrun\DAVrun.exe";  // Путь к программе, которая будет запускаться каждые "START_TIMER" минут
        public const string SERVICE_NAME = "DAVrun";                                // Название службы
        public const string DISPLAY_NAME = "DAVrun Service";                        // Отображаемое имя службы
        public const string DESCRIPTION = "Служба каждый установленный интервал времени запускает программу \"DAVrun\", для авторазвёртывания приложений."; // Описание службы
        
        //Логирование
        private const string PATH_LOG = @"C:\ProgramData\DAVrun";   // Путь, куда сохранять лог файл
        private const int MAX_LOG_SIZE = 50000;                     // Максимальный размер (в байтах) лог файла для ротации. (Установлено 50 Кбайт)
        private const int MAX_LOG_FILES = 1;                        // Максимальное количество лог файлов для хранения. Архивные файлы с суффиксом "_0"

        private Timer timer;    // Таймер, по истечению которого будет запускаться программа "DAVrun"


        public DAVrunService()
        {
            InitializeComponent();  // Инициализация компонентов

            ServiceName = SERVICE_NAME; // Устанавливаем название службы
            CanStop = true;             // разрешаем остановку службы
            CanPauseAndContinue = true; // Разрешием приостановить и возобновить службу
        }

        //Метод для запуска службы
        protected override void OnStart(string[] args)
        {
            try
            {
                WriteToLogFile("Служба установлена и успешно запущена.");  // Пишем лог

                // Таймер для начального запуска через 10 секунд
                Timer initialTimer = new Timer(10000); // 10 секунд (10 000 миллисекунд)
                initialTimer.Elapsed += (sender, e) =>
                {
                    initialTimer.Stop();            // Останавливаем начальный таймер
                    initialTimer.Dispose();         // Уничтожаем начальный таймер
                    OnTimerElapsed(sender, null);   // Выполняем начальный запуск
                    StartMainTimer();               // Запускаем основной таймер
                };
                initialTimer.Start();
            }
            catch (Exception ex)
            {
                WriteToLogFile($"Не удалось установить или запустить службу: {ex.Message}");    // Пишем ошибку в лог
            }
        }

        //Метод для остановки службы
        protected override void OnStop()
        {
            try
            {
                WriteToLogFile("Служба успешно остановлена и удалена.");  // Пишем лог

                timer.Stop();   // Останавливаем таймер
            }
            catch (Exception ex)
            {
                WriteToLogFile($"Не удалось остановить или удалить службу: {ex.Message}"); // Пишем ошибку в лог
            }
        }

        // Таймер для последующих запусков через каждые "START_TIMER" минут
        private void StartMainTimer()
        {
            timer = new Timer(START_TIMER);     // Создаём таймер
            timer.Elapsed += OnTimerElapsed;    // Добавляем обработчик событий
            timer.Start();                      // Запускаем таймер
        }

        //Функция обработчик для запуска программы по таймеру
        private void OnTimerElapsed(object sender, ElapsedEventArgs e)
        {
            try
            {
                // Завершаем все запущенные процессы DAVrun.exe
                var processes = Process.GetProcessesByName("DAVrun");
                foreach (var process in processes)
                {
                    try
                    {
                        process.Kill();
                        process.WaitForExit(); // Ожидание завершения процесса
                        // WriteToLogFile($"Процесс {process.Id} DAVrun.exe был завершён.");   // ДЛЯ ОТЛАДКИ
                    }
                    catch (Exception ex)
                    {
                        WriteToLogFile($"Не удалось завершить процесс {process.Id}: {ex.Message}");
                    }
                }

                // Запускаем новый процесс DAVrun.exe
                Process.Start(PROGRAM_PATH);
                // WriteToLogFile("Новая копия DAVrun.exe была успешно запущена."); // ДЛЯ ОТЛАДКИ
            }
            catch (Exception ex)
            {
                WriteToLogFile($"Ошибка при запуске программы: {ex.Message}");
            }
        }

        //Функция логирования
        public static void WriteToLogFile(string message)
        {
            try
            {
                //Проверяем путь создания лога, если отсутствует, создаём его
                if (!Directory.Exists(PATH_LOG))
                {
                    Directory.CreateDirectory(PATH_LOG);
                }

                string logFilePath = Path.Combine(PATH_LOG, "log_SERVICE-DAVrun.txt");  // Формируем полный путь к файлу лога

                if (File.Exists(logFilePath) && new FileInfo(logFilePath).Length >= MAX_LOG_SIZE)   // Проверяем размер файла лога, и если он превышаетзаданный размер
                {
                    RotateLogFiles();   // Вызываем метод ротации логов
                }

                using (StreamWriter sw = new StreamWriter(logFilePath, true, Encoding.GetEncoding("windows-1251"))) // Создаём экземпляр "StreamWriter", для добавления строки с датой и временем в лог
                {
                    sw.WriteLine($"{DateTime.Now:dd.MM.yy в HH:mm:ss}: {message}"); // Создаём формат времени "дд.мм.гг в чч:мм:сс"
                }
            }
            catch (Exception)
            {
                //В случае ошибки логирования ничего не делаем
            }
        }

        //Функция ротации лог файлов
        public static void RotateLogFiles()
        {
            //Удаляем самый старый лог, если превышен лимит по объёму файла
            string oldestLogFile = Path.Combine(PATH_LOG, $"log_SERVICE-DAVrun_{MAX_LOG_FILES - 1}.txt");
            if (File.Exists(oldestLogFile))
            {
                File.Delete(oldestLogFile);
            }

            //Перемещаем архивные логи на одну позицию вверх
            for (int i = MAX_LOG_FILES - 1; i > 0; i--)
            {
                string oldFile = Path.Combine(PATH_LOG, $"log_SERVICE-DAVrun_{i - 1}.txt");
                string newFile = Path.Combine(PATH_LOG, $"log_SERVICE-DAVrun_{i}.txt");

                if (File.Exists(oldFile))
                {
                    File.Move(oldFile, newFile);
                }
            }

            //Перемещаем текущий лог в первый архивный лог
            string currentLogFile = Path.Combine(PATH_LOG, "log_SERVICE-DAVrun.txt");
            string newLogFile = Path.Combine(PATH_LOG, "log_SERVICE-DAVrun_0.txt");

            if (File.Exists(currentLogFile))
            {
                File.Move(currentLogFile, newLogFile);
            }
        }
    }


    //Класс для управления службой
    public static class ServiceInstaller
    {
        [DllImport("advapi32.dll", SetLastError = true, CharSet = CharSet.Unicode)] // Подключаем библиотеку для взаимодействия с функцией "ChangeServiceConfig2"
        private static extern bool ChangeServiceConfig2(IntPtr hService, uint dwInfoLevel, ref SERVICE_DESCRIPTION lpInfo);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]    // Определяем макет структуры
        
        //Создаём структуру для работы со службой
        private struct SERVICE_DESCRIPTION
        {
            public string lpDescription;
        }

        //Функция для установки и запуска службы
        public static void InstallAndStart(string serviceName, string displayName, string description, string fileName)
        {
            using (var process = new Process()) // Создаём новый процесс для работы с командной строкой
            {
                process.StartInfo.FileName = "sc.exe";  // Указываем на встроенную утилиту "sc" для работы с процессами

                //Создание службы от учётной записи "LocalSystem" (она же "СИСТЕМА" в Русском языке) для получения повышенных прав
                process.StartInfo.Arguments = $"create {serviceName} binPath= \"{fileName}\" displayName= \"{displayName}\" start= auto obj= LocalSystem";   // Формируем команды для создания новой службы
                process.Start();        // Запускаем процесс на выполнение
                process.WaitForExit();  // Ожидаем завершения выполнения команды

                //Добавление описания службы
                process.StartInfo.Arguments = $"description {serviceName} \"{description}\"";
                process.Start();        // Запускаем процесс на выполнение
                process.WaitForExit();  // Ожидаем завершения выполнения команды

                //Настройка параметров восстановления службы
                process.StartInfo.Arguments = $"failure \"{serviceName}\" reset= 0 actions= restart/60000/restart/60000/restart/60000"; // Перезапуск службы 3 раза с интервалом в 1 минуту
                process.Start();        // Запускаем процесс на выполнение
                process.WaitForExit();  // Ожидаем завершения выполнения команды

                //Запуск службы
                process.StartInfo.Arguments = $"start {serviceName}";
                process.Start();        // Запускаем процесс на выполнение
                process.WaitForExit();  // Ожидаем завершения выполнения команды
            }
        }


        //Функция для остановки и удаления службы
        public static void Uninstall(string serviceName)
        {
            using (var process = new Process()) // Создаём новый процесс для работы с командной строкой
            {
                process.StartInfo.FileName = "sc.exe";  // Указываем на встроенную утилиту "sc" для работы с процессами

                //Остановка службы
                process.StartInfo.Arguments = $"stop {serviceName}";
                process.Start();        // Запускаем процесс на выполнение
                process.WaitForExit();  // Ожидаем завершения выполнения команды

                //Удаление службы
                process.StartInfo.Arguments = $"delete {serviceName}";
                process.Start();        // Запускаем процесс на выполнение
                process.WaitForExit();  // Ожидаем завершения выполнения команды
            }
        }
    }
}
