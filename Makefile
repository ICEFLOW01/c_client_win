Target=Client
Sources=client.c
L_Linux=gcc  -o $(Target)  $(Sources) -lpthread
L_Windows=i686-w64-mingw32-gcc -o $(Target).exe $(Sources) -lws2_32 -D_MINGW_ 


client:
	$(L_Linux) 
	$(L_Windows)
