/*
	Простая консольная программа для установки сертифкатов с автоматическим подтверждением всплывающих окон безопасности.

	Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
	Копия лицензии: https://opensource.org/licenses/MIT

	Copyright (c) 2024 Otto
	Автор: Otto
	Версия: 04.07.24
	GitHub страница:  https://github.com/Otto17/DAVrun
	GitFlic страница: https://gitflic.ru/project/otto/davrun

	г. Омск 2024
*/


using System;                           // Библиотека предоставляет доступ к базовым классам и функциональности .NET Framework
using System.Diagnostics;               // Библиотека для работы с отладкой
using System.Runtime.InteropServices;   // Библиотека для взаимодействия с неуправляемым кодом
//using System.Management;
using System.Windows.Forms;             // Библиотека используется для создания графического пользовательского интерфейса (GUI) в приложениях Windows

namespace Admin_DAVrun
{
    internal static class Program
    {
        /// Главная точка входа для приложения.

        //Определяется внешний метод "IsDebuggerPresent" из библиотеки "kernel32.dll", который используется для проверки наличия отладчика
        [DllImport("kernel32.dll", SetLastError = true)]    // Загружаем библиотеку "kernel32.dll" для использования функции "IsDebuggerPresent()"
        [return: MarshalAs(UnmanagedType.Bool)]             // Преобразуем значение от функции "IsDebuggerPresent()" в тип bool
        static extern bool IsDebuggerPresent();             // Функция проверки отладчика в процессе, в котором она вызывается

        [STAThread] // Атрибут [STAThread] указывает, что COM-поток однопоточный (STA)
        static void Main()
        {
            /*Проверяем, прикреплен ли отладчик к процессу с помощью двух методов:
            1) "Debugger.IsAttached" - встроенный метод в ".NET", проверяющий, прикреплен ли отладчик.
            2) "IsDebuggerPresent()" - вызов внешнего метода из библиотеки Windows API, который также проверяет наличие отладчика.
            */

            if (Debugger.IsAttached || IsDebuggerPresent()) // Если отладчик обнаружен
            {
           //     Environment.Exit(1);    // Тогда сразу завершаем работу программы с кодом ошибки "1"
            }

            //if (IsVirtualMachine()) // Если обнаружена виртуальная машина
            //{
            //    Environment.Exit(1);    // Тогда сразу завершаем работу программы с кодом ошибки "1"
            //}


            //Настройка и запуск приложения в штатном режиме
            Application.EnableVisualStyles();                       // Включаем визуальные стили для приложения (улучшая его внешний вид)
            Application.SetCompatibleTextRenderingDefault(false);   // Устанавливаем совместимость рендеринга текста
            Application.Run(new Connect());                         // Запускает основную форму приложения "Connect", что делает приложение GUI видимым и готовым к взаимодействию с пользователем
        }

        //Функция защиты от запуска на некоторых виртуальных машинах
        //static bool IsVirtualMachine()
        //{
        //    //Создаём экземпляр класса "ManagementObjectSearcher" для поиска информации о компьютерной системе
        //    using (var searcher = new ManagementObjectSearcher("Select * from Win32_ComputerSystem"))
        //    {
        //        foreach (var item in searcher.Get())    // Перебираем в цикле все найденные объекты
        //        {
        //            string manufacturer = item["Manufacturer"].ToString().ToLower();    // Производитель виртуальной машины (в нижнем регистре)
        //            string model = item["Model"].ToString().ToLower();                  // Модель виртуальной машины (в нижнем регистре)

        //            if ((manufacturer.Contains("microsoft") && model.Contains("virtual")) ||
        //                manufacturer.Contains("vmware") ||
        //                model.Contains("virtualbox"))
        //            {
        //                return true;    // Если любая из виртуальных машин обнаружена, возвращаем истину
        //            }
        //        }
        //    }
        //    return false;   // Иначе ничего не нашли и разрешаем программе работать
        //}
    }
}
