#define BASE_URL "https://www1.gogoanime.cm"
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
static void regex(char *content, char *match_pattern, char *memcpy_result, int s_eps);
static size_t regex_animes(char *buffer, size_t itemsize, size_t nitems, int *ignorethis);
static void curl_urls(char *url, void *call_func, long int *verbose);
static void search_anime(void);
static void search_eps();
static void anime_selection();

/* variables */
static char search[45];
static char search_url[128];
static char eps_url[sizeof(search_url)];
static char regex_result[sizeof(search)];
static char regex_eps_result[sizeof(search)];
char *search_results[MAX_MATCHES][sizeof(search)];
/* static char *search_results[MAX_MATCHES]; */
static char anime_id[30];
static long int verbosely = 1L;
int *s_eps;

int animes_found = 0;

void
printexit(char *str)
{
	fprintf(stderr, "%s\n", str);
	exit(0);
}

void
regex(char *content, char *match_pattern, char *memcpy_result, int s_eps)
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

	if(reti == 0)
	{
		if(s_eps == 0)
		{
			memcpy(memcpy_result, content + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
			strncpy((char *)search_results[animes_found], memcpy_result, 1024);
			animes_found++;
		}
		else
		{
			memcpy(memcpy_result, content + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
			printf("%s\n", regex_eps_result);
		}
		/* append results to search_results array */
		/* char str[sizeof(search)]; */
		/* snprintf(str, sizeof(str), "%s\n", regex_result); */
		/* strncat((char *)search_results, str, sizeof(str)); */
	}

	/* Free memory allocated to the pattern buffer by regcomp() */
	regfree(&regex);
}

/* function implementations */
size_t
regex_animes(char *buffer, size_t itemsize, size_t nitems, int *ignorethis)
{
	size_t bytes = itemsize * nitems;

	char pattern[] = "\"/category/([^\"]*)\" title=\"([^\"]*)\".*";

	regex((char *)buffer, pattern, regex_result, 0);

	return bytes;
}

size_t
regex_eps(char *buffer, size_t itemsize, size_t nitems, int *ignorethis)
{
	size_t bytes = itemsize * nitems;

	/* printf("%s\n", buffer); */
	char pattern[] = "ep_start = '\''([0-9]*)'\'' ep_end = '\''([0-9]*)'\''.*";

	regex((char *)buffer, pattern, regex_eps_result, 1);

	return bytes;
}

void
curl_urls(char *url, void *call_func, long int *verbose)
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
	curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose);
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
	for(int i = 0; i < strnlen(search, sizeof(search)); i++)
	{
		if(isspace(search[i]))
			search[i] = '-';
	}

	/* search_url */
	snprintf(search_url, sizeof(search_url), "%s/search.html?keyword=%s", BASE_URL, search);

	curl_urls(search_url, regex_animes, (long int *)verbosely); /* 1L means verbose is active, 0L means its off */

	/* clear the screen */
	if(!verbosely)
		printf("\e[1;1H\e[2J");
}

void
search_eps()
{
	snprintf(eps_url, sizeof(eps_url), "%s/category/%s", BASE_URL, anime_id);

	curl_urls(eps_url, regex_eps, (long int *)verbosely); /* 1L means verbose is active, 0L means its off */
}

void
anime_selection()
{
	int anime_number;

	printf("%s\n", "Found");
	for(int j = 0; j < animes_found; j++)
	{
		/* printf("%s\n", search_results[anime_number]); */
		printf("[%i] %s\n", j+1, (char *)search_results[j]);
	}
	printf("%s\n", "Enter number");
	scanf("%d", &anime_number);
	/* the first anime in the list is search_results[0], so if we enter 1 we will get the second anime in the list. this fixes that */
	anime_number--;

	strncpy(anime_id, search_results[anime_number], sizeof(anime_id));
}

int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		printexit("ani-cli-re: ani-cli rewritten in C");
	else if (argc != 1)
		printexit("usage: ani-cli-re [-v]");

	/* start process */
	search_anime();

	/* if there is no result exit */
	if(strnlen((char *)search_results, sizeof(search_results)) <= 0)
		printexit("No Search Results");

	/* printf("%s\n", search_results[3]); */
	anime_selection();
	search_eps();
	return 0;
}

