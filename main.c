#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
/* Curl Library */
#include <curl/curl.h>

/* function declarations */
static size_t regex_animes(char *buffer, size_t itemsize, size_t nitems, void* ignorethis);
static void curl_urls(char *url, void *call_func,long int *verbosely);
static void search_anime(void);
static void search_eps(char *anime_id);

/* variables */
static char search[30];
static const char base_url[] = "https://gogoanime.cm";
static char search_url[49];
static char eps_url[sizeof(search_url)];
/* char search_results[200]; */
char anime_id[30];
char call_func;
char verbosely;

/* function implementations */
size_t /* i couldnt find a good way to do this so i skipped this functionality. normally we will set anime_id here */
regex_animes(char *buffer, size_t itemsize, size_t nitems, void* ignorethis)
{
	size_t bytes = itemsize * nitems;


	printf("%s\n",buffer);
	strncpy(anime_id, "tokyo-ghoul", sizeof(anime_id));

	/* printf("New chunk (%zu bytes)\n", bytes); */
	/* printf("%s\n", buffer); */
	return bytes;
}

void
curl_urls(char *url, void *call_func,long int *verbosely)
{
	/* Curl */
	CURL *curl = curl_easy_init();
	CURLcode res;
	/* check curl */
	if(!curl)
	{
		fprintf(stderr,"Failed to initialize curl!\n");
		exit(-1);
	}

	/* for debugging */
	curl_easy_setopt(curl, CURLOPT_VERBOSE, verbosely);
	/* send all data to this function  */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_func);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	res = curl_easy_perform(curl);
	/* check */
	if(res != CURLE_OK)
	{
		fprintf(stderr,"Could not fetch %s. Error: %s\n",url,curl_easy_strerror(res));
		exit(-2);
	}

	/* we are done */
	curl_easy_cleanup(curl);
}

void
search_anime(void)
{
	/* Get user input */
	printf("%s\n","Enter anime name: ");
	fgets(search, sizeof(search), stdin);

	/* Replace spaces with "-" */
	for(int i = 0; i < strlen(search); i++)
	{
		if(search[i] == ' ')
			search[i] = '-';
	}

	/* search_url */
	snprintf(search_url, sizeof(search_url),"%s/search.html?keyword=%s", base_url, search);

	curl_urls(search_url,regex_animes, 1L); /* 1L means verbose is active, 0L means its off */

}

void
search_eps(char *anime_id)
{
	snprintf(eps_url, sizeof(eps_url),"%s/category/%s", base_url, anime_id);

	curl_urls(eps_url, regex_animes, 1L); /* 1L means verbose is active, 0L means its off */
}

int
main()
{
	search_anime();
	search_eps(anime_id);
	return 0;
}

