cl -c -Ox -MD -I\projects\cpp\downhill\include -DLOCALEDIR=\"/usr/local/share/locale\" -DGNULOCALEDIR=\"/usr/local/share/locale\" -DLOCALE_ALIAS_PATH=\"/usr/local/share/locale:.\" -DHAVE_CONFIG_H -I..  -I. -I../intl -I../lib  %1 %2 %3
gcc -c -O2 -DLOCALEDIR=\"/usr/local/share/locale\" -DGNULOCALEDIR=\"/usr/local/share/locale\" -DLOCALE_ALIAS_PATH=\"/usr/local/share/locale:.\" -DHAVE_CONFIG_H -I..  -I. -I../intl -I../lib  %1 %2 %3