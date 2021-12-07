#define BASE_URL "https://www1.gogoanime.cm"
#define MAX_MATCHES 20
#define PLAYER "mpv"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
/* Curl Library */
#include <curl/curl.h>
/* Regex */
#include <regex.h>

/* function declarations */
static void printexit(char *str);
void regex(char *content, char *match_pattern, char *memcpy_result, int job);
static size_t regex_animes(char *buffer, size_t itemsize, size_t nitems, int *ignorethis);
static size_t regex_eps(char *buffer, size_t itemsize, size_t nitems, int *ignorethis);
static void curl_urls(char *url, void *call_func, long int *verbose);
static void search_anime(void);
static void search_eps(void);
static void anime_selection(void);
static void episode_selection(void);
static void open_episode(void);

/* variables */
static char search[45];
/* url variables */
static char search_url[128];
static char eps_url[sizeof(search_url)];
static char episode_url[sizeof(search_url)];
static char embedded_video_urls[MAX_MATCHES][2048];
static char get_links[MAX_MATCHES][sizeof(embedded_video_urls)];
/* result variables */
static char regex_result[sizeof(search)];
static char regex_eps_result[sizeof(search)];
static char regex_embedded_video_url_results[sizeof(search_url)];
static char regex_get_links_results[sizeof(search_url)];
char *search_results[MAX_MATCHES][sizeof(search)];
int eps_results;
/* other variables */
static char anime_id[30];
static long int verbosely = 1L;
int animes_found = 0;
int embedded_video_urls_found = 0;
static int episode_number;
int download = 0;

void
printexit(char *str)
{
	fprintf(stderr, "%s\n", str);
	exit(0);
}

void
regex(char *content, char *match_pattern, char *memcpy_result, int job)
{
	regex_t regex;
	int reti;
	regmatch_t matches[MAX_MATCHES];

	/* Compile regular expression */
	reti = regcomp(&regex, match_pattern, REG_EXTENDED);
	if (reti) {
		fprintf(stderr, "%s\n", "Could not compile regex");
		exit(1);
	}

	/* Execute regular expression */
	reti = regexec(&regex, content, MAX_MATCHES, matches, 0);

	if(reti == 0)
	{
		memcpy(memcpy_result, content + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
		if(job == 0)
		{
			strncpy((char *)search_results[animes_found], memcpy_result, sizeof(search_results));
			animes_found++;
		}
		else if(job == 1)
			/* return episode number as an integer */
			sscanf(memcpy_result, "%d", &eps_results);
		else if(job == 2)
		{
			strncpy((char *)embedded_video_urls[embedded_video_urls_found], memcpy_result, sizeof(embedded_video_urls));
			embedded_video_urls_found++;
		}
		else if(job == 3)
		{
			strncpy((char *)get_links, memcpy_result, sizeof(get_links));
			printf("%s\n", get_links);
		}
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

	char pattern[] = "<a href=\"#\" class=\"active\".*ep_end = '([0-9]*)'.*";

	regex((char *)buffer, pattern, regex_eps_result, 1);

	return bytes;
}

size_t
regex_embedded_video_url(char *buffer, size_t itemsize, size_t nitems, int *ignorethis)
{
	size_t bytes = itemsize * nitems;

	/* printf("%s\n", buffer); */
	/* char pattern[] = "ep_start = '\''([0-9]*)'\'' ep_end = '\''([0-9]*)'\''.*"; */
	char pattern[] = "rel=\"100\" data-video=\"([^\"]*)\".*";

	regex((char *)buffer, pattern, regex_embedded_video_url_results, 2);

	return bytes;
}

size_t
regex_get_links(char *buffer, size_t itemsize, size_t nitems, int *ignorethis)
{
	size_t bytes = itemsize * nitems;

	/* printf("%s\n", buffer); */
	/* char pattern[] = "ep_start = '\''([0-9]*)'\'' ep_end = '\''([0-9]*)'\''.*"; */
	char pattern[] = ".*sources:.*(https[^\']*).*";

	regex((char *)buffer, pattern, regex_get_links_results, 3);

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

}

void
search_eps(void)
{
	snprintf(eps_url, sizeof(eps_url), "%s/category/%s", BASE_URL, anime_id);
	curl_urls(eps_url, regex_eps, (long int *)verbosely); /* 1L means verbose is active, 0L means its off */
}

void
anime_selection(void)
{
	/* clear the screen */
	if(!verbosely)
		printf("\e[1;1H\e[2J\n");

	int anime_number;

	printf("%s\n", "Found");
	for(int j = 0; j < animes_found; j++)
	{
		printf("[%i] %s\n", j+1, (char *)search_results[j]);
	}
	printf("%s\n", "Enter number");
	while(scanf("%d", &anime_number) != 1 || (int)(anime_number) != anime_number || anime_number > animes_found || anime_number == 0)
	{
		printf("%s\n", "Invalid number entered");
		printf("%s\n", "Enter number");
		if(scanf("%d", &anime_number) != 1 || (int)(anime_number) != anime_number || anime_number > animes_found || anime_number == 0)
			printexit("Invalid number entered");
	}

	/* the first anime in the list is search_results[0], so if we enter 1 we will get the second anime in the list. this fixes that */
	anime_number--;

	strncpy(anime_id, (char *)search_results[anime_number], sizeof(anime_id));
}

void
episode_selection(void)
{
	int ep_choice_start = 1;

	/* clear the screen */
	if(!verbosely)
		printf("\e[1;1H\e[2J\n");

	printf("%s [%i-%i]\n", "Choose episode", ep_choice_start, eps_results);
	while(scanf("%d", &episode_number) != 1 || (int)(episode_number) != episode_number || episode_number > eps_results || episode_number == 0)
	{
		printf("%s\n", "Invalid number entered");
		printf("%s [%i-%i]\n", "Choose episode", ep_choice_start, eps_results);
		if(scanf("%d", &episode_number) != 1 || (int)(episode_number) != episode_number || episode_number > eps_results || episode_number == 0)
			printexit("Invalid number entered");
	}
}

void
open_episode(void)
{
	char temp_embedded_video_url[sizeof(embedded_video_urls)];
	char temp_video_url[sizeof(embedded_video_urls)];
	char player_command[2524];

	/* clear the screen */
	if(!verbosely)
		printf("\e[1;1H\e[2J\n");

	printf("%s %i\n", "Getting data for episode", episode_number);

	snprintf(episode_url, sizeof(episode_url), "%s/%s-episode-%i", BASE_URL, anime_id, episode_number);
	curl_urls(episode_url, regex_embedded_video_url, (long int *)verbosely);

	/* get links */
	if(strstr(embedded_video_urls[0], "https:") == NULL)
		snprintf(temp_embedded_video_url, sizeof(temp_embedded_video_url), "https:%s", embedded_video_urls[0]);
	printf("%s\n", embedded_video_urls[0]);
	curl_urls(temp_embedded_video_url, regex_get_links, (long int *)verbosely);
	/* printf("%s\n", get_links); */

	strncpy(temp_video_url, (char *)get_links, sizeof(temp_video_url));
	if(download == 0)
		snprintf(player_command, sizeof(player_command), "setsid -f %s --http-header-fields=\"Referer: %s\" \"%s\"", PLAYER, temp_embedded_video_url, temp_video_url);
	else if(download == 1)
		snprintf(player_command, sizeof(player_command), "ffmpeg -headers \"Referer: %s\" -i \"%s\" -c copy \"%s-%i.mkv\"", temp_embedded_video_url, temp_video_url, anime_id, episode_number);

	puts(player_command);

	system(player_command);
}

int
main(int argc, char *argv[])
{
	int opt;

	while((opt = getopt(argc, argv, "dvh")) != -1)
	{
		switch(opt)
		{
			case 'd':
				download = 1;
				break;
			case 'v':
				printexit("ani-cli-re: ani-cli rewritten in C");
				break;
			case 'h':
				printexit("usage: ani-cli-re [-v] [-d]");
				break;
			default:
				break;
		}
	}

	/* start process */
	search_anime();
	/* if there is no result exit */
	if(strnlen((char *)search_results, sizeof(search_results)) <= 0)
		printexit("No Search Results");

	anime_selection();
	search_eps();
	episode_selection();
	open_episode();
	return 0;
}

