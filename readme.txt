Для платы LDM-uROB-K1986BE1Q1
https://ldm-systems.ru/product/21103

[2021-04-13]
1. После исправления ошибок 
   $ sudo ping 10.1.2.70 -s 1440 -c 100000 -A -q
   PING 10.1.2.70 (10.1.2.70) 1440(1468) bytes of data.

   --- 10.1.2.70 ping statistics ---
   100000 packets transmitted, 100000 received, 0% packet loss, time 128ms
   rtt min/avg/max/mdev = 0.539/1.059/1.473/0.036 ms, ipg/ewma 1.090/1.057 ms


[2021-04-09]
1. Работает ping. Адрес 10.1.2.70. Размер буфера до 1440 байт.

   $ sudo ping 10.1.2.70 -s 1440 -c 100000 -A -q
     PING 10.1.2.70 (10.1.2.70) 1440(1468) bytes of data.
     --- 10.1.2.70 ping statistics ---
     100000 packets transmitted, 99975 received, +5 corrupted, 0.025% packet loss, time 925ms
     rtt min/avg/max/mdev = 0.640/1.114/1.481/0.042 ms, ipg/ewma 1.148/1.124 ms


[2021-19-03]
1. Передаёт пакет с нулями и строчкой "ETHERNET DATA FROM 1986VE1T"
