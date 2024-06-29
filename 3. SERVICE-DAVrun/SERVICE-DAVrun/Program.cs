/*
    Программа представляет собой службу в Windows, которая запускает другую указанную в коде программу через установленный интервал времени.

    Программа, при запуске с ключом "-is" (insstal - start) установит и запустит службу с именем "SERVICE-DAVrun.exe".
    При запуске с ключом "-sd" (stop - delete) служба остановится и удалится.

    При попытке запустить программу без ключей всплывёт сообщение "Сбой при запуске службы Windows".


	Данная программа является свободным программным обеспечением, распространяющимся по лицензии MIT.
	Копия лицензии: https://opensource.org/licenses/MIT

	Copyright (c) 2024 Otto
	Автор: Otto
	Версия: 20.06.24
	GitHub страница:  https://github.com/Otto17/DAVrun
	GitFlic страница: https://gitflic.ru/project/otto/davrun

	г. Омск 2024
*/


using System.ServiceProcess;    // Библиотека для работаты со службами Windows
using System.Diagnostics;       // Библиотека для работы с процессами и системными ресурсами компьютера

namespace SERVICE_DAVrun
{
    internal static class Program
    {
        /// <summary>
        /// Главная точка входа для приложения.
        /// </summary>
        static void Main(string[] args) // Главная функция принимает строку на вход
        {
            if (args.Length == 1)   // Если получили любые аргументы на вход
            {
                if (args[0] == "-is")   // Если аргумент равен "-is" (insstal - start)
                {
                    string exePath = Process.GetCurrentProcess().MainModule.FileName;                                                               // Получаем путь к исполняемому файлу текущего процесса
                    ServiceInstaller.InstallAndStart(DAVrunService.SERVICE_NAME, DAVrunService.DISPLAY_NAME, DAVrunService.DESCRIPTION, exePath);   // Устанавливаем и запускаем службу с помощью метода "InstallAndStart"
                }
                else if (args[0] == "-sd")  // Если аргумент равен "-sd" (stop - delete)
                {
                    ServiceInstaller.Uninstall(DAVrunService.SERVICE_NAME); // останавливаем и удаляем службу с помощью метода "Uninstall"
                }
                else
                {
                    //Иначе, если аргумент не соответствует любому из условий, пишем как ошибку в лог
                    DAVrunService.WriteToLogFile("Запуск программы с неверным ключом. Программа будет работать только как служба:\n\nКлюч \"-is\" для установки и запуска службы\nКлюч \"-sd\" для остановки и удаления службы\n");  // Пишем ошибку в лог
                }
            }
            else
            {
                ServiceBase.Run(new DAVrunService());   // Если кол-во аргументов не равно 1, запускается служба "DAVrunService"
            }
        }
    }
}
