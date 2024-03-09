#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "sqlite3.h"
#include <pthread.h>

/* portul folosit */
#define PORT 2024

/* codul de eroare returnat de anumite apeluri */
extern int errno;

int verifyLogin(char user[100], char pass[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT user, pass FROM users", -1, &stmt, 0);
	char userDB[256], passDB[256];
	int response = 0;
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		strcpy(userDB,sqlite3_column_text(stmt, 0));
		strcpy(passDB,sqlite3_column_text(stmt, 1));
		if(strcmp(userDB,user) == 0 && strcmp(passDB,pass) == 0)
		{
			response = 1;
		}
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);
	return response;
}
struct log
{
		int client;
		int id;
		char user[100];
		char mesaj[1000];
};


void initLU(int a[100])
{
	for(int i = 0; i < 100; i++)
		a[i] = 0;
}

int getID(char user[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT id, user FROM users", -1, &stmt, 0);
	char userDB[256];
	int idDB, idR;
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		idDB = sqlite3_column_int(stmt, 0);
		strcpy(userDB,sqlite3_column_text(stmt, 1));
	
		if(strcmp(userDB,user) == 0)
		{
			idR = idDB;
		}
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);
	return idR;	
}
 
void commLogin(char user[100], char pass[100], char msg[100])
{
	bzero(user, 100);
	bzero(pass,100);
	user[0] = '\0';
	pass[0] = '\0';
	char copie[100];
	strcpy(copie, msg);
	char* cuv = strtok(msg, " ");
	int contor = 0;
	while(cuv != NULL)
	{
		if(contor == 1)
		{
			strcpy(user, cuv);
		}
		else if(contor == 2)
		{
			strcpy(pass, cuv);
		}
		contor++;
		cuv = strtok(NULL, " ");
	}
	pass[strlen(pass) - 1] = '\0';
}


int verifyUser(char user[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT user FROM users", -1, &stmt, 0);
	char userDB[256];
	int response = 0;
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		strcpy(userDB,sqlite3_column_text(stmt, 0));
		if(strcmp(userDB,user) == 0)
		{
			response = 1;
		}
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);
	return response;
}
int countUsers()
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT user FROM users", -1, &stmt, 0);

	int response = 0;
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		 response = response + 1;
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);
	return response;
}

void insertNewUser(char user[100], char pass[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "INSERT INTO users VALUES(?,?,?)", -1, &stmt, 0);

	sqlite3_bind_int(stmt, 1, countUsers() + 1);
	sqlite3_bind_text(stmt, 2, user, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, pass, -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
    sqlite3_close(db);

}

void getUserSendMsg(char msg[100], char user[100])
{
	/*bzero(user, 100);
	bzero(pass,100);*/
	user[0] = '\0';
	char copie[100];
	strcpy(copie, msg);
	char* cuv = strtok(copie, " ");
	int contor = 0;
	while(cuv != NULL)
	{
		if(contor == 1)
		{
			strcpy(user, cuv);
		}
		contor++;
		cuv = strtok(NULL, " ");
	}
	/*pass[strlen(pass) - 1] = '\0';*/
	if(contor == 2)
	{
		user[0] = '\0';
	}
}

void getMessage(char msg[100], char rMsg[100])
{
	rMsg[0] = '\0';
	char copie[100];
	strcpy(copie, msg);
	char* cuv = strtok(copie, " ");
	int contor = 0;
	char text[25][25];
	while(cuv != NULL)
	{
		strcpy(text[contor], cuv); 
		contor = contor + 1;
		cuv = strtok(NULL, " ");
	}
	strcpy(rMsg, text[2]);
	for(int i = 3; i < contor; i++)
	{
		strcat(rMsg, " ");
		strcat(rMsg, text[i]);
	}
	rMsg[strlen(rMsg) - 1] = '\0';	
}

void insertMsgOffline(char userSEND[100], char userRECV[100], char msg[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "INSERT INTO newMsg VALUES(?,?,?)", -1, &stmt, 0);

	sqlite3_bind_text(stmt, 1, userSEND, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, userRECV, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, msg, -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int countNewMsg(char user[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT recv FROM newMsg", -1, &stmt, 0);
	char userRECV[100];
	int response = 0;
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		strcpy(userRECV,sqlite3_column_text(stmt, 0));
		if(strcmp(user,userRECV) == 0)
		{
			response = response + 1;
		}
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);
	return response;	
}

void getInbox(char user[100], char rasp[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT send, msg FROM newMsg WHERE recv=?", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, user, -1, SQLITE_TRANSIENT);

	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		strcat(rasp,sqlite3_column_text(stmt, 0));
		strcat(rasp,": ");
		strcat(rasp,sqlite3_column_text(stmt, 1));
		if(rasp[strlen(rasp) - 1] == '\n')
			rasp[strlen(rasp) - 1] = '\0';
		strcat(rasp,"\n");
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);

	rasp[strlen(rasp) - 1] = '\0';
}

void deleteFromNewMsg(char user[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "DELETE FROM newMsg WHERE recv=?", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, user, -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
    sqlite3_close(db);

	 
}
void getUserHistory(char msg[100], char user[100])
{
	user[0] = '\0';
	char copie[100];
	strcpy(copie, msg);
	char* cuv = strtok(msg, " ");
	int contor = 0;
	while(cuv != NULL)
	{
		if(contor == 1)
		{
			strcpy(user, cuv);
		}
		contor++;
		cuv = strtok(NULL, " ");
	}
	user[strlen(user) - 1] = '\0';
	if(contor > 2)
	{
		user[0] = '\0';
	}
	
}

int countMsgHistory(char uS[100], char uR[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT send, recv FROM history", -1, &stmt, 0);
	char userSEND[100];
	char userRECV[100];
	int response = 0;
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{

		strcpy(userSEND,sqlite3_column_text(stmt, 0));
		strcpy(userRECV,sqlite3_column_text(stmt, 1));
		if(strcmp(uR,userRECV) == 0 && strcmp(uS,userSEND) == 0)
		{
			response = response + 1;
		}
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);
	return response;	
}

void insertMsgHistory (char userS[100], char userR[100], char mess[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "INSERT INTO history VALUES(?,?,?,?)", -1, &stmt, 0);

	sqlite3_bind_text(stmt, 1, userS, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, userR, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, mess, -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 4, countMsgHistory(userS,userR) + 1);

	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void getHistoryConv(char usS[100], char usR[100], char rasp[100])
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT send, recv, msg, count FROM history", -1, &stmt, 0);
	int nrmesaje = 0;

	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		if((strcmp(usS, sqlite3_column_text(stmt, 0)) == 0 && strcmp(usR, sqlite3_column_text(stmt, 1)) == 0) || (strcmp(usS, sqlite3_column_text(stmt, 1)) == 0 && strcmp(usR, sqlite3_column_text(stmt, 0)) == 0 ) )
		{
			nrmesaje = nrmesaje + 1;
			char numar[100];
			sprintf(numar,"%d",sqlite3_column_int(stmt, 3));

			strcat(rasp,sqlite3_column_text(stmt, 0));
			if(strcmp(usS, sqlite3_column_text(stmt, 1)) == 0)
			{
				strcat(rasp,":");
				strcat(rasp,"(");
				strcat(rasp,numar);
				strcat(rasp,") ");
			}
			else if(strcmp(usS, sqlite3_column_text(stmt, 0)) == 0)
			{
				strcat(rasp,": ");
			}
			strcat(rasp,sqlite3_column_text(stmt, 2));

			if(rasp[strlen(rasp) - 1] == '\n')
				rasp[strlen(rasp) - 1] = '\0';
			strcat(rasp,"\n");
		}
		
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);

	rasp[strlen(rasp) - 1] = '\0';
	if(nrmesaje == 0)
	{
		rasp[0] = '\0';
	}	
}

void getReplyData(char msg[1000], char id[100], char msgR[1000], char userTO[100])
{
	bzero(userTO, 100);
	bzero(msgR,1000);
	userTO[0] = '\0';
	msgR[0] = '\0';
	bzero(id, 100);
	id[0] = '\0';

	char copie[100];
	strcpy(copie, msg);
	char textR[25][25];
	char* cuv = strtok(msg, " ");
	int contor = 0;
	while(cuv != NULL)
	{
		strcpy(textR[contor], cuv); 
		contor = contor + 1;
		cuv = strtok(NULL, " ");
	}
	strcpy(userTO, textR[1]);
	strcpy(id, textR[2]);

	strcpy(msgR, textR[3]);
	for(int i = 4; i < contor; i++)
	{
		strcat(msgR, " ");
		strcat(msgR, textR[i]);
	}

	msgR[strlen(msgR) - 1] = '\0';
	if(contor < 4)
	{
		userTO[0] = '\0';
		msgR[0] = '\0';
		id[0] = '\0';
	}	
}
int verifyReply(char userS[100], char userR[100], int idMsg)
{
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT send, recv, count FROM history", -1, &stmt, 0);
	char sendDB[100];
	char recvDB[100];
	int idDB = 0;
	int response = 0;
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		strcpy(sendDB,sqlite3_column_text(stmt, 0));
		strcpy(recvDB,sqlite3_column_text(stmt, 1));
		idDB = sqlite3_column_int(stmt, 2);
		if(strcmp(sendDB,userS) == 0 && strcmp(recvDB, userR) == 0 && idDB == idMsg)
		{
			response = 1;
		}
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);
	return response;	
}
void ReplyMsgFromDB(char rezultat[100], char userS[100], char userR[100], int nrMsg)
{
	rezultat[0] = '\0';
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int rc;
	rc = sqlite3_open("data.db", &db);
	if(rc != SQLITE_OK)
	{
		fprintf(stderr, "Cannot open database");
		sqlite3_close(db);
	}
	sqlite3_prepare_v2(db, "SELECT send, recv, msg, count FROM history", -1, &stmt, 0);

	char sendDB[100];
	char recvDB[100];
	int idDB = 0;
	while(sqlite3_step(stmt) == SQLITE_ROW)
	{
		strcpy(sendDB,sqlite3_column_text(stmt, 0));
		strcpy(recvDB,sqlite3_column_text(stmt, 1));
		idDB = sqlite3_column_int(stmt, 3);
		if(strcmp(sendDB,userS) == 0 && strcmp(recvDB, userR) == 0 && idDB == nrMsg)
		{
			strcpy(rezultat, sqlite3_column_text(stmt, 2));
		}
	}
	sqlite3_finalize(stmt);
    sqlite3_close(db);

}

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

 
struct log* loggedUsers;

void raspunde(void *arg)
{
    int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	int client = tdL.cl;
			struct log msg;		//mesajul primit de la client
    		struct log msgrasp;        //mesaj de raspuns pentru client

			int bytes;
			bzero (msg.mesaj, 100);
			while((bytes = read (client, &msg, sizeof(struct log))))
			{
				msgrasp.id = msg.id;
				strcpy(msgrasp.user, msg.user);
				/*printf("mesaj primit: %s", msg.mesaj);*/
				printf("id:%d, logged:%d, user:%s, client:%d\n",msg.id,loggedUsers[msg.id].id,msg.user,loggedUsers[msg.id].client);
					/* s-a realizat conexiunea, se astepta mesajul */ 	
					
				/*printf ("[server]Asteptam mesajul...\n");*/
				fflush (stdout);

				/*printf ("[server]Mesajul a fost receptionat...%s\n", msg.mesaj);*/
				/*printf("stare id_3:%d",loggedUsers[3]);
				printf("stare id_1: %d",loggedUsers[1]);*/

				if(strstr(msg.mesaj, "login") != NULL)
				{
					/*printf("id:%d, logged:%d",msg.id,loggedUsers[msg.id]);*/
					if(msg.id == 0)
					{					
						char user[100], pass[100];
						commLogin(user, pass, msg.mesaj);
						
						/*printf("[Server]aceasta este comanda login\n");*/
						bzero(msgrasp.mesaj,1000);
						/*strcat(msgrasp,"aceasta este comanda login\n");*/

						if(user[0] != '\0' && pass[0] != '\0')
						{
							if(verifyLogin(user,pass) == 1)
							{
								int id = getID(user);
								/*printf("id:%d, stare inainte de login:%d",id,loggedUsers[id]);*/
								if(loggedUsers[id].id == 0)
								{
									loggedUsers[id].client = client;
									loggedUsers[id].id = id;
									msgrasp.id = id;
									strcpy(msgrasp.user, user);
									strcat(msgrasp.mesaj, user);
									strcat(msgrasp.mesaj, ": te-ai logat cu succes");
									strcat(msgrasp.mesaj,"\n");

									char inboxUser[100];
									strcpy(inboxUser, user);
									int numarMsg = countNewMsg(inboxUser);
									char numarMsgChar[100];
									/*printf("Numar mesaje noi: %d\n", numarMsg);*/
									sprintf(numarMsgChar,"%d",numarMsg);
									/*printf("Numar msj noi dupa fct: %s\n", numarMsgChar);*/
									/*printf("[server]aceasta este comanda inbox\n");*/
									/*bzero(msgrasp.mesaj,100);*/
									/*strcat(msgrasp.mesaj,"aceasta este comanda inbox");*/
									strcat(msgrasp.mesaj,"Ai ");
									strcat(msgrasp.mesaj, numarMsgChar);
									strcat(msgrasp.mesaj, " mesaje noi!");
								}
								else if(loggedUsers[id].id != 0)
								{
									strcat(msgrasp.mesaj, "Utilizator deja conectat!");
								}
							}
							else if(verifyLogin(user,pass) == 0)
							{
								strcat(msgrasp.mesaj, "Eroare, acest utilizator nu exista, te poti inregistra folosind comanda register");
							}
						
						}
						else 
							strcat(msgrasp.mesaj, "Eroare, structura: login user parola");
					}
					else if(msg.id != 0)
					{
						bzero(msgrasp.mesaj, 1000);
						strcpy(msgrasp.mesaj, "Utilizator deja conectat!");
					}
				}
				else if(strcmp(msg.mesaj, "logout\n") == 0)
				{
					if(msg.id != 0)
					{
						loggedUsers[msg.id].id = 0;
						msgrasp.id = 0;
						msgrasp.user[0] = '\0';
						printf("[server]aceasta este comanda logout\n");
						bzero(msgrasp.mesaj,1000);
						strcat(msgrasp.mesaj,"logout realizat cu succes!");
					}
					else if(msg.id == 0)
					{
						printf("[server]Aceasta este comanda logout\n");
						bzero(msgrasp.mesaj,100);
						strcat(msgrasp.mesaj,"Pentru a apela logout trebuie sa fii logat!!");
					}

					
				}
				else if(strstr(msg.mesaj, "send_msg") != NULL)
				{
					if(msg.id != 0)
					{
						char userSEND[100], userRECV[100];
						userRECV[0] = '\0';
						strcpy(userSEND, msg.user);
						printf("userSEND:%s\n",userSEND);

						getUserSendMsg(msg.mesaj, userRECV);
						printf("userRECV:%s\n",userRECV);

						if(userRECV[0] != '\0')
						{
							char userMSG[100];
							userMSG[0] = '\0';
							getMessage(msg.mesaj, userMSG);
							printf("Mesaj:%s\n",userMSG);

							if(verifyUser(userRECV) == 1)
							{
								if(strcmp(userSEND,userRECV))
								{
									int idRECV = getID(userRECV);
									if(loggedUsers[idRECV].id == 0)
									{
										insertMsgOffline(userSEND,userRECV, userMSG);
										insertMsgHistory(userSEND,userRECV, userMSG);
										bzero(msgrasp.mesaj,1000);
										strcat(msgrasp.mesaj,"Mesaj trimis cu succes!");
									}
									else if(loggedUsers[idRECV].id != 0)
									{
										struct log msgTrimis;
										msgTrimis.id = loggedUsers[idRECV].id;
										printf("status user primit:%d",loggedUsers[idRECV].id);

										bzero(msgTrimis.mesaj,1000);
										strcpy(msgTrimis.mesaj, userSEND);
										strcat(msgTrimis.mesaj, ": ");
										strcat(msgTrimis.mesaj, userMSG);
										strcpy(msgTrimis.user,userRECV);
										///msgTrimis.client = loggedUsers[idRECV].client;
										insertMsgHistory(userSEND,userRECV, userMSG);

										write (loggedUsers[idRECV].client, &msgTrimis, sizeof(struct log));
										bzero(msgrasp.mesaj,1000);
										strcpy(msgrasp.mesaj, "Mesaj trimis cu succes!");
										/*printf("[server]aceasta este comanda send message\n");
										strcat(msgrasp.mesaj,"aceasta este comanda send message");*/
									}
								}
								else if(strcmp(userSEND,userRECV) == 0)
								{
									bzero(msgrasp.mesaj,1000);
									strcat(msgrasp.mesaj, "Nu iti poti trimite singur mesaje!!");
								}
								
								
							}
							else if(verifyUser(userRECV) == 0)
							{
								bzero(msgrasp.mesaj, 1000);
								strcat(msgrasp.mesaj, "Nu exista utilizatorul: ");
								strcat(msgrasp.mesaj, userRECV);
							}
							
						}
						else if(userRECV[0] == '\0')
						{
							bzero(msgrasp.mesaj,1000);
							strcat(msgrasp.mesaj, "Strctura comenzii: send_msg destinatar 'mesaj'");
						}
						
					}
					else if(msg.id == 0)
					{
						printf("[server]Aceasta este comanda send_msg\n");
						bzero(msgrasp.mesaj,1000);
						strcat(msgrasp.mesaj,"pentru a trimite un mesaj trebuie sa fii logat!!");
					}
					
				}
				else if(strstr(msg.mesaj, "history") != NULL)
				{
					if(msg.id != 0)
					{
						char userHist[100];
						getUserHistory(msg.mesaj, userHist);
						printf("user history:%s\n",userHist);
						if(userHist[0] != '\0')
						{
							if(strcmp(msg.user,userHist) != 0)
							{
								if(verifyUser(userHist) == 1)
								{
									
									printf("[server]aceasta este comanda history\n");
									bzero(msgrasp.mesaj,1000);
									getHistoryConv(msg.user, userHist, msgrasp.mesaj);
									if(msgrasp.mesaj[0] == '\0')
									{
										bzero(msgrasp.mesaj,1000);
										strcat(msgrasp.mesaj,"Nu ai istoric cu acest utilizator!");
									}
									/*strcat(msgrasp.mesaj,"aceasta este comanda history");*/
								}
								else if(verifyUser(userHist) == 0)
								{
									printf("[server]aceasta este comanda history\n");
									bzero(msgrasp.mesaj,1000);
									strcat(msgrasp.mesaj,"Nu exista acest utilizator!");
								}
							}
							else if(strcmp(msg.user,userHist) == 0)
							{
								printf("[server]aceasta este comanda history\n");
								bzero(msgrasp.mesaj,1000);
								strcat(msgrasp.mesaj,"Nu poti vedea istoricul cu tine!");
							}
							
							
						}
						else if(userHist[0] == '\0')
						{
							bzero(msgrasp.mesaj,1000);
							strcat(msgrasp.mesaj,"Eroare, structura comenzii: history user");
						}
						
					}
					else if(msg.id == 0)
					{
						bzero(msgrasp.mesaj, 1000);
						strcat(msgrasp.mesaj,"pentru a vedea istoricul unei conversatii trebuie sa fii logat!");
					}
					
				}
				else if(strcmp(msg.mesaj, "inbox\n") == 0)
				{
					if(msg.id != 0)
					{
						char vinboxUser[100];
						strcpy(vinboxUser, msg.user);

						printf("[server]aceasta este comanda view_inbox\n");
						bzero(msgrasp.mesaj,1000);
						if(countNewMsg(vinboxUser) != 0)
						{
							getInbox(vinboxUser,msgrasp.mesaj);
							deleteFromNewMsg(vinboxUser);
						}
						else if(countNewMsg(vinboxUser) == 0)
						{
							strcat(msgrasp.mesaj, "nu ai mesaje noi!");
						} 
					}
					else if(msg.id == 0)
					{
						printf("[server]Aceasta este comanda inbox");
						bzero(msgrasp.mesaj,1000);
						strcat(msgrasp.mesaj, "Pentru a accesa inboxul trebuie sa fii logat!");
					}
					
				}
				else if(strcmp(msg.mesaj, "view_inbox\n") == 0)
				{
					if(msg.id != 0)
					{
						char vinboxUser[100];
						strcpy(vinboxUser, msg.user);

						printf("[server]aceasta este comanda view_inbox\n");
						bzero(msgrasp.mesaj,1000);
						if(countNewMsg(vinboxUser) != 0)
						{
							getInbox(vinboxUser,msgrasp.mesaj);
							deleteFromNewMsg(vinboxUser);
						}
						else if(countNewMsg(vinboxUser) == 0)
						{
							strcat(msgrasp.mesaj, "nu ai mesaje noi!");
						}
						/*strcat(msgrasp.mesaj,"aceasta este comanda view inbox");*/
						
						
					}
					else if(msg.id == 0)
					{
						printf("[server]Aceasta este comanda view_inbox");
						bzero(msgrasp.mesaj,1000);
						strcat(msgrasp.mesaj, "Pentru a accesa inboxul trebuie sa fii logat!");
					}
					
				}
				else if(strstr(msg.mesaj, "reply") != NULL)
				{
					if(msg.id != 0)
					{
						int idReply;
						idReply = 0;
						char idReplyChar[100];
						idReplyChar[0] = '\0';
						char replyMSG[1000];
						replyMSG[0] = '\0';
						char replyUser[100];
						replyUser[0] = '\0';
						getReplyData(msg.mesaj, idReplyChar, replyMSG, replyUser);
						/*printf("id inainte de sprintf:%s",idReplyChar);*/

						if(replyUser[0] != '\0')
						{
							idReply = atoi(idReplyChar);

							printf("user:%s, idmsg:%d, msg:%s\n",replyUser, idReply, replyMSG);
							if(verifyUser(replyUser) == 1)
							{
								if(verifyReply(replyUser, msg.user, idReply) == 1)
								{
									printf("[server]Mesajul EXISTA\n");
									int replyUserID = getID(replyUser);
									if(loggedUsers[replyUserID].id != 0)
									{
										struct log repTrimis;
										repTrimis.id = loggedUsers[replyUserID].id;
										printf("status user primit:%d\n",loggedUsers[replyUserID].id);
										char dbMsg[1000];
										ReplyMsgFromDB(dbMsg, replyUser, msg.user, idReply);
										printf("msg la care rasp:%s\n",dbMsg);

										bzero(repTrimis.mesaj,1000);
										strcpy(repTrimis.mesaj, msg.user);
										strcat(repTrimis.mesaj, " a raspuns la '");
										strcat(repTrimis.mesaj,dbMsg);
										strcat(repTrimis.mesaj,"': ");
										strcat(repTrimis.mesaj, replyMSG);
										strcpy(repTrimis.user,replyUser);

										insertMsgHistory(msg.user,replyUser, replyMSG);

										write (loggedUsers[replyUserID].client, &repTrimis, sizeof(struct log));
									}
									else if(loggedUsers[replyUserID].id == 0)
									{
										char dbMsgOFF[1000];
										ReplyMsgFromDB(dbMsgOFF, replyUser, msg.user, idReply);
										char offlineReply[100];
										offlineReply[0] = '\0';
										strcat(offlineReply, "reply la'");
										strcat(offlineReply, dbMsgOFF);
										strcat(offlineReply, "': ");
										strcat(offlineReply, replyMSG);

										insertMsgOffline(msg.user, replyUser, offlineReply);

										insertMsgHistory(msg.user,replyUser, replyMSG);

										bzero(msgrasp.mesaj,1000);
										strcat(msgrasp.mesaj,"Mesaj trimis cu succes!");
									}

									bzero(msgrasp.mesaj,1000);
									strcat(msgrasp.mesaj,"Raspuns trimis cu succes!");
								}
								else if(verifyReply(replyUser, msg.user, idReply) == 0)
								{
									printf("[server]Mesajul NU EXISTA\n");
									bzero(msgrasp.mesaj,1000);
									strcat(msgrasp.mesaj,"Mesajul la care vrei sa raspunzi nu exista");
								}
							}
							else if(verifyUser(replyUser) == 0)
							{
								bzero(msgrasp.mesaj,1000);
								strcat(msgrasp.mesaj,"Utilizatorul caruia vrei sa-i raspunzi nu exista!");
							}
							
							
						}
						else if(replyUser[0] == '\0')
						{
							bzero(msgrasp.mesaj,1000);
							strcat(msgrasp.mesaj, "Eroare, structura comenzii: reply user id_mesaj mesaj!!");
						}
						
					}
					else if(msg.id == 0)
					{
						printf("[server]aceasta este comanda reply\n");
						bzero(msgrasp.mesaj,1000);
						strcat(msgrasp.mesaj,"Pentru a raspunde unui mesaj trebuie sa fii logat!");
					}
					
				}
				else if(strstr(msg.mesaj, "register") != NULL)
				{
					if(msg.id == 0)
					{
						char user[100], pass[100];
						commLogin(user, pass, msg.mesaj);

						printf("[server]aceasta este comanda register\n");
						bzero(msgrasp.mesaj,1000);
						/*strcat(msgrasp,"aceasta este comanda register");*/

						if(user[0] != '\0' && pass[0] != '\0')
						{
							if(verifyUser(user) == 1)
							{
								strcat(msgrasp.mesaj, "Acest nume de utilizator este deja folosit!");
							}
							else if(verifyUser(user) == 0)
							{
								insertNewUser(user, pass);
								strcat(msgrasp.mesaj, "Utilizator inregistrat cu succes!");
							}
						
						}
						else 
							strcat(msgrasp.mesaj, "Eroare, structura: register user parola");		
					}
					else if(msg.id != 0)
					{
						bzero(msgrasp.mesaj, 1000);
						strcat(msgrasp.mesaj, "Nu poti inregistra un utilizator daca esti logat!");
					}

				}
				else if(strcmp(msg.mesaj, "help\n") == 0)
				{
					printf("[server]aceasta este comanda help\n");
					bzero(msgrasp.mesaj,1000);
					strcat(msgrasp.mesaj,"Comenzi disponibile:\n");
					strcat(msgrasp.mesaj,"login (login user parola) pentru a te conecta\n");
					strcat(msgrasp.mesaj,"logout pentru a te deconecta\n");
					strcat(msgrasp.mesaj,"register (register user parola) pentru a te inregistra\n");
					strcat(msgrasp.mesaj,"send_msg (send_msg user mesaj) pentru a trimite un mesaj\n");
					strcat(msgrasp.mesaj,"history (history username) pentru istoricul conversatiei\n");
					strcat(msgrasp.mesaj,"inbox sau view_inbox pentru a vedea mesajele primite cand erai offline\n");
					strcat(msgrasp.mesaj,"reply (reply user id_mesaj mesaj) pentru a raspunde la un anumit mesaj\n");
					strcat(msgrasp.mesaj,"quit pentru a inchide aplicatia");
				}
				else if(strcmp(msg.mesaj, "quit\n") == 0)
				{

						loggedUsers[msg.id].id = 0;
						msgrasp.id = 0;
						msgrasp.user[0] = '\0';
						/*loggedUsers[msg.id].client = 0;*/
						printf("[server]aceasta este comanda quit\n");
						bzero(msgrasp.mesaj,1000);
						strcat(msgrasp.mesaj,"Ai iesit din Offline Messenger!");
					 
				}
				else
				{
					printf("[server]Comanda neidentificata, tastati comanda help pentru mai multe detalii\n");
					bzero(msgrasp.mesaj,1000);
					strcat(msgrasp.mesaj,"Comanda neidentificata, tastati comanda help pentru mai multe detalii");
				}

				printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp.mesaj);


				/* returnam mesajul clientului */
				if (write (client, &msgrasp, sizeof(struct log)) <= 0)
				{
					perror ("[server]Eroare la write() catre client.\n");
					continue;		/* continuam sa ascultam */
				}
				else
					printf ("[server]Mesajul a fost trasmis cu succes.\n");
				
				/* am terminat cu acest client, inchidem conexiunea */
			}
    		 
    		close (client);
    		exit(0);

}

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close (tdL.cl);
		return(NULL);	
  		
};

int main()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    int sd;			//descriptorul de socket
	pthread_t th[100];
		int ti = 0;
	
    /* crearea unui socket */
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }

	/* utilizarea optiunii SO_REUSEADDR */
  	int on=1;
  	setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof (server));
    bzero(&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if(bind(sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if(listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }

	loggedUsers = mmap(NULL, 1000*sizeof(struct log), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	bzero(loggedUsers, 1000*sizeof(struct log));

    /* servim in mod concurent clientii... */
    while (1)
    {
    	int client;
    	int length = sizeof (from);
		thData *td;
    	printf("[server]Asteptam la portul %d...\n",PORT);
    	fflush(stdout);

    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept(sd, (struct sockaddr *) &from, &length);
		
    	/* eroare la acceptarea conexiunii de la un client */
    	if(client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}
		td=(struct thData*)malloc(sizeof(struct thData));	
		td->idThread=ti++;
		td->cl=client;

	pthread_create(&th[ti], NULL, &treat, td);	
    }				
}				