#define BASE_URL "https://gogoanime.cm"
#define MAX_MATCHES 20

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
/* Curl Library */
#include <curl/curl.h>
/* Regex */
#include <regex.h>


/* function declarations */
static void regex(char *content, char *match_pattern);
static size_t regex_animes(char *buffer, size_t itemsize, size_t nitems, void* ignorethis);
static void curl_urls(char *url, void *call_func, long int *verbosely);
static void search_anime(void);
static void search_eps(char *anime_id);

/* variables */
static char search[45];
static char search_url[128];
static char eps_url[sizeof(search_url)];
char result[1024];
char anime_id[30];
char call_func;
long int verbosely;


void
regex(char *content, char *match_pattern)
{
	regex_t regex;
	int reti;

	/* Compile regular expression */
	reti = regcomp(&regex, match_pattern, REG_EXTENDED);
	if (reti) {
    fprintf(stderr, "Could not compile regex\n");
    exit(1);
	}

	regmatch_t matches[MAX_MATCHES];
	/* Execute regular expression */
	reti = regexec(&regex, content, MAX_MATCHES, matches, 0);
	if (!reti) {
	  puts("Match");
	}
	else if (reti == REG_NOMATCH) {
    puts("No match");
	}
	else {
    regerror(reti, &regex, content, sizeof(content));
    fprintf(stderr, "Regex match failed: %s\n", content);
    exit(1);
	}

	if(reti == 0)
	{
		memcpy(result, content + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
		printf("%s\n", result);
	}

	/* Free memory allocated to the pattern buffer by regcomp() */
	regfree(&regex);
}

/* function implementations */
size_t /* i couldnt find a good way to do this so i skipped this functionality. normally we will set anime_id here */
regex_animes(char *buffer, size_t itemsize, size_t nitems, void* ignorethis)
{
	size_t bytes = itemsize * nitems;

	char pattern[] = "^[[:space:]]*<a href=\"/category/([^\"]*)\" title=\"([^\"]*)\".*";

	/* char pattern[] = "\"/category/([^\"]*)\" title=\"([^\"]*)\".*"; */
	/* char pattern[] = "\"/category/tokyo-*"; */

	regex((char *)buffer, pattern);

	/* printf("%s\n",buffer); */

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
		fprintf(stderr, "%s\n", "Failed to initialize curl!");
		exit(-1);
	}

	/* for debugging */
	curl_easy_setopt(curl, CURLOPT_VERBOSE, verbosely);
	/* send all data to this function  */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_func);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	res = curl_easy_perform(curl);
	/* check */
	if(res != CURLE_OK)
	{
		fprintf(stderr, "Could not fetch %s. Error: %s\n", url, curl_easy_strerror(res));
		exit(-2);
	}

	/* we are done */
	curl_easy_cleanup(curl);
}

void
search_anime(void)
{
	/* Get user input */
	printf("%s\n", "Enter anime name:");
	fgets(search, sizeof(search), stdin);

	/* Replace spaces with "-" */
	for(int i = 0; i < strlen(search); i++)
	{
		if(isspace(search[i]))
			search[i] = '-';
	}

	/* search_url */
	snprintf(search_url, sizeof(search_url), "%s/search.html?keyword=%s", BASE_URL, search);

	curl_urls(search_url, regex_animes, (long int *)1L); /* 1L means verbose is active, 0L means its off */
}

void
search_eps(char *anime_id)
{
	snprintf(eps_url, sizeof(eps_url), "%s/category/%s", BASE_URL, anime_id);

	curl_urls(eps_url, regex_animes, (long int *)1L); /* 1L means verbose is active, 0L means its off */
}

int
main()
{
	search_anime();
	/* strncpy(anime_id, "tokyo-ghoul", sizeof(anime_id)); */
	/* search_eps(anime_id); */
	return 0;
}

