#!/bin/bash


# Данный скрипт является свободным программным обеспечением, распространяющимся по лицензии MIT.
# Копия лицензии: https://opensource.org/licenses/MIT

# Copyright (c) 2024 Otto
# Автор: Otto
# Версия: 16.09.24
# GitHub страница:  https://github.com/Otto17/DAVrun
# GitFlic страница: https://gitflic.ru/project/otto/davrun

# г. Омск 2024


# Останавливаем выполнение скрипта в случае ошибки
set -e

# Проверяем, запущен ли скрипт от sudo или root
if [ "$(id -u)" != "0" ]; then
    echo "ЗАПУСТИТЕ скрипт через 'sudo' или от 'root'!"
    echo "Если используете Debian, рекомендую зайти от рута: 'su - root'"
    exit 1
fi

# Определяем пользователя системы
# Пользователь определится корректно, независимо от того, как запущен скрипт: от рута или через утилиту sudo
USER_SYSTEM="${SUDO_USER:-$USER}"
if [ -z "$USER_SYSTEM" ]; then
  USER_SYSTEM=$(whoami)
fi



# -/-/-/- НАСТРОЙКИ -/-/-/- #

# Заменить "user" на свой СЛОЖНЫЙ И ДЛИННЫЙ логин (для доступа к WebDAV)
: ${WEBDAV_USERNAME:=user}

# Заменить "password" на свой СЛОЖНЫЙ И ДЛИННЫЙ пароль (для доступа к WebDAV)
: ${WEBDAV_PASSWORD:=password}

# Заменить информацию для создания сертификата
# Срок действия сертификата (в днях)
DAYS=3650
# Код страны
C="RU"
# Страна
ST="Russia"
# Город
L="Omsk"
# Организация (слеши экранируют кавычки, если они нужны)
O="OOO 'Organization'"
# Подразделение
OU="OOO 'Organization'"
# IP адрес или домен (ЭТО САМОЕ ВАЖНОЕ ПОЛЕ ПРИ СОЗДАНИИ СЕРТА)
# Если хоть один символ будет неверный, "DAVrun" не будет проходить проверку подлинности сервера
CN="77.77.77.77"

# Максимальное кол-во одновременных подключений к Apache (по умолчанию 150)
# Изменять на то значение, на скольких компьютерах будет установлена программа "DAVrun", если меньше 150 ПК, то можно оставить по умолчанию
MAXVALUE=150

# IP-адреса, которые нужно исключить из блокировки Fail2Ban (РАЗДЕЛЯТЬ IP АДРЕСА ПРОБЕЛАМИ)
IGNORE_IP=""
# Примеры:
# Можно так (диапазон IP и ещё конкретный IP)
#IGNORE_IP="192.168.0.17/24 203.0.113.42"
# или так (просто IP). Если оставить поле пустое (как по умолчанию), то никакие IP не исключаются
#IGNORE_IP="192.168.1.27"
# Посмотреть заблокированные IP командой:
# "sudo fail2ban-client status sshd" или "sudo fail2ban-client status webdav" (смотря в какой секции бан)
# Исключить заблокированные IP командой:
# "sudo fail2ban-client set sshd unbanip 192.168.1.27" или "sudo fail2ban-client set webdav unbanip 192.168.0.17" (смотря в какой секции бан)



# -/-/-/- ОБНОВЛЕНИЕ И УСТАНОВКА -/-/-/- #

# Обновляем систему
apt update && apt full-upgrade -y

# Устанавливаем "Apache" (сервер), "OpenSSL" (для создания сертификата), "Expect" (для облегчения автоматизации), "fail2ban" (для защиты от перебора паролей), acl (список управления доступом) и ufw (Брандмауэр)
apt install -y apache2 openssl expect fail2ban acl ufw rsyslog



# -/-/-/- СЕРВИС УПРАВЛЕНИЯ ЛОГАМИ -/-/-/- #

# Устанавливаем службу управления логами на автозапуск
systemctl enable rsyslog

# И запускаем её
systemctl start rsyslog



# -/-/-/- СОЗДАНИЕ СЕРТИФИКАТА -/-/-/- #
# Закрытый ключ
KEYOUT="${USER_SYSTEM}.key"
# Открытый ключ
OUT="${USER_SYSTEM}.crt"

# Сборка строки SUBJ
SUBJ="/C=${C}/ST=${ST}/L=${L}/O=${O}/OU=${OU}/CN=${CN}"

# Создаём самоподписанный сертификат с использованием утилиты "expect"
expect -c "
set timeout -1
spawn openssl req -x509 -nodes -days ${DAYS} -newkey rsa:2048 -keyout /etc/ssl/private/${KEYOUT} -out /etc/ssl/certs/${OUT} -subj \"${SUBJ}\"
expect eof
"

echo "Самоподписанный сертификат успешно создан"
echo "Закрытый ключ: /etc/ssl/private/${KEYOUT}"
echo "Открытый ключ: /etc/ssl/certs/${OUT}"

# Устанавливаем правильные (строгие) права на сертификаты
chmod 600 /etc/ssl/certs/${OUT}
chmod 600 /etc/ssl/private/${KEYOUT}



# -/-/-/- НАСТРОЙКА Apache -/-/-/- #

# Создаём бэкап (на всякий случай) из стандартного конфига (если бэкап уже был ранее создан, то не перезаписываем его)
[ ! -e /etc/apache2/sites-available/000-default.conf_bak ] && mv /etc/apache2/sites-available/000-default.conf /etc/apache2/sites-available/000-default.conf_bak

# Изменяем кол-во одновременных подключений к Apache
sed -i "s/^\(\s*MaxRequestWorkers\s*\).*$/\1$MAXVALUE/" /etc/apache2/mods-available/mpm_worker.conf

# Создаём новый конфиг с перенаправлением с http на https, а так же перенаправлением на "/webdav"
cat <<EOF | tee /etc/apache2/sites-available/000-default.conf
Alias /webdav /var/www/webdav

<VirtualHost *:80>

  ServerAdmin webmaster@localhost
  DocumentRoot /var/www/webdav

  # Местоположение для хранения логов
  ErrorLog /var/log/apache2/error.log
  CustomLog /var/log/apache2/access.log combined

  # Разрешаем перезапись URL-адресов для перенаправления запросов
  RewriteEngine On

  # Правило для переадресации с HTTP на HTTPS
  RewriteCond %{HTTPS} off
  RewriteRule ^ https://%{HTTP_HOST}%{REQUEST_URI} [R=301,L]

  # Правило для переадресации с корневого URL на "/webdav"
  RewriteCond %{REQUEST_URI} ^/$
  RewriteRule ^(.*)$ https://%{HTTP_HOST}/webdav [R=301,L]

</VirtualHost>

EOF

# Удаляем стандартную папку "html"
rm -R /var/www/html

# Скрываем версию сервера и модулях
# Путь к файлу конфигурации Apache
config_file="/etc/apache2/apache2.conf"

# Если хотя бы одно строки "ServerTokens Prod" и "ServerSignature Off" нет, тогда добавляем их в конец конфига
if ! grep -q "^ServerTokens Prod" "$config_file" || ! grep -q "^ServerSignature Off" "$config_file"; then
    echo -e "\n\n# Скрытие версии сервера и модулей\nServerTokens Prod\nServerSignature Off\n" >> "$config_file"
fi

# Цикл, в котором перебираются модули для веб-сервера Apache
# Каждый модуль включается командой "a2enmod".
for mod in rewrite dav dav_fs dav_lock ssl auth_digest; do
a2enmod ${mod}
done

# Перезапускаем Apache
systemctl restart apache2



# -/-/-/- НАСТРОЙКА WebDAV -/-/-/- #

# Создаём конфиг файл с настройками WebDAV
cat <<EOF | tee /etc/apache2/sites-available/webdav.conf
Alias /webdav /var/www/webdav

<VirtualHost *:443>

  ServerAdmin webmaster@localhost
  DocumentRoot /var/www/webdav

  # Местоположение для хранения логов
  ErrorLog /var/log/apache2/error.log
  CustomLog /var/log/apache2/access.log combined

  # Вкл. использование SSL
  SSLEngine on
  SSLCertificateFile    /etc/ssl/certs/${OUT}
  SSLCertificateKeyFile /etc/ssl/private/${KEYOUT}

  # Разрешаем перезапись URL-адресов для перенаправления запросов
  RewriteEngine On

  # Правило для переадресации с HTTP на HTTPS
  RewriteCond %{HTTPS} off
  RewriteRule ^ https://%{HTTP_HOST}%{REQUEST_URI} [R=301,L]

  # Правило для переадресации с корневого URL на "/webdav"
  RewriteCond %{REQUEST_URI} ^/$
  RewriteRule ^(.*)$ https://%{HTTP_HOST}/webdav [R=301,L]


  <Directory /var/www/webdav>
      # Включаем WebDAV
      DAV On
      # Требуем использование SSL
      SSLRequireSSL
      # Устанавливает тип аутентификации - "Digest"
      AuthType Digest
      # Имя для аутентификации
      AuthName webdav
      # Указываем файл с сохранённым пользователем и паролем для авторизации в WebDAV
      AuthUserFile /etc/apache2/webdav.passwd
      # Требуем ввод логина и пароля
      Require valid-user
  </Directory>

</VirtualHost>

EOF

# Создаём каталог "webdav"
mkdir -p /var/www/webdav

# Назначаем владельца каталога и группу "www-data"
chown -R www-data:www-data /var/www/webdav

# Установка прав через ACL (Access Control Lists)
setfacl -d -m u::rwx,g::rwx,o::rx /var/www/webdav
setfacl -d -m u:www-data:rwx,g:www-data:rwx /var/www/webdav
setfacl -m u:www-data:rwx,g:www-data:rwx /var/www/webdav

# Установка setgid-бита:
chmod g+s /var/www/webdav

# Добавляем текущего пользователя в группу "www-data"
usermod -aG www-data ${USER_SYSTEM}

# Активируем виртуальный хост "webdav" в веб-сервере Apache
a2ensite webdav

# Создаём файл с дайджестом паролем с помощью утилиты для автоматизации "expect"
expect -c "
set timeout -1
spawn htdigest -c /etc/apache2/webdav.passwd webdav ${WEBDAV_USERNAME}
expect \"New password: \"
send \"${WEBDAV_PASSWORD}\n\"
expect \"Re-type new password: \"
send \"${WEBDAV_PASSWORD}\n\"
expect eof
"

# Перезапускаем Apache
systemctl restart apache2



# -/-/-/- НАСТРОЙКА Fail2Ban -/-/-/- #

# Ставим Fail2Ban на автозапуск
systemctl enable fail2ban

# Создаём конфигурацию в fail2ban для SSH и WebDAV
tee  /etc/fail2ban/jail.local <<EOF
[sshd]
enabled = true
port = ssh
filter = sshd
logpath = /var/log/auth.log
# Баним IP-адреса на 24 часа после 5 неудачных попыток в течение 10 минут
maxretry = 5
findtime = 600
bantime = 86400
ignoreip = $IGNORE_IP

[webdav]
enabled = true
port = http,https
filter = webdav
logpath = /var/log/apache2/error.log
# Баним IP-адреса на 24 часа после 5 неудачных попыток в течение 10 минут
maxretry = 5
findtime = 600
bantime = 86400
ignoreip = $IGNORE_IP

EOF

# Создание фильтра для webdav
 tee /etc/fail2ban/filter.d/webdav.conf <<EOF
[Definition]
failregex = ^.*\[auth_digest:error\].* \[client <HOST>:[0-9]+\] AH01790: user .* in realm .* not found: .*
ignoreregex =

EOF

# Перезапускаем Fail2Ban
systemctl restart fail2ban



# -/-/-/- НАСТРОЙКА SSH -/-/-/- #
# Принудительно запрещаем подключение от root по SSH
sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin no/' /etc/ssh/sshd_config

# Перезапускаем демон SSH
systemctl restart sshd



# -/-/-/- НАСТРОЙКА UFW -/-/-/- #

# Отключаем "IPv6" в брандмауэре
sed -i 's/IPV6=yes/IPV6=no/' /etc/default/ufw

# Восстанавливаем политики по умолчанию (запрещаем все входящие и разрешаем все исходящие)
ufw default deny incoming
ufw default allow outgoing

# Разрешаем SSH
ufw allow ssh

# Разрешаем https
ufw allow https

# Включаем брандмауэр (без запроса на подтверждение)
ufw --force enable

# Показываем статус UFW для наглядности
ufw status

# Очищаем локальный кеш от скаченных пакетов
apt clean

echo WebDAV сервер настроен и защищён!
