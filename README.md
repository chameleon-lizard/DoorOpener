Komodo - это приложение для того чтобы открывать дверь. Для использования, 
необходимо в папку с программой положить файл config со следующим содержанием:

config:

  SIP: <Ссылка на стрим с камеры>
  
  OIP: <IP-адрес сервера>
  
  OLP: <Логин> <Пароль>



После запуска приложения (make run или make && ./komodo) открывается окно со
стримом с камеры и с кнопкой открытия двери в левом нижнем углу. При нажатии
отправляется запрос:

ALLOWPASS 1 ANONYMOUS IN

После этого дверь открывается для одного прохода.
