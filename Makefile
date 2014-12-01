build:
	gcc -DDEBUG -g -I./kii_sources -I/usr/local/include -I//usr/local/Cellar/curl/7.38.0/include/curl/ main.c kii_sources/*.c -L/usr/local/Cellar/curl/7.38.0/lib -L/usr/local/lib -l jansson -l curl -o thingdemo
