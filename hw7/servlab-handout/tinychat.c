/*
 * tinychat.c - [Starting code for] a web-based chat server.
 *
 * Based on:
 *  tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *      GET method to serve static and dynamic content.
 *   Tiny Web server
 *   Dave O'Hallaron
 *   Carnegie Mellon University
 */
#include "csapp.h"
#include "dictionary.h"
#include "more_string.h"

void doit(int fd);
dictionary_t *read_requesthdrs(rio_t *rp);
void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *d);
void parse_query(const char *uri, dictionary_t *d);
void serve_form(int fd, dictionary_t* query);
void clienterror(int fd, char *cause, char *errnum,
         char *shortmsg, char *longmsg);
static void print_stringdictionary(dictionary_t *d);
void serve_login(int fd, dictionary_t* query);
void appendToConvo(char* topic, char* user, char* content);
  
dictionary_t* convos; // Each value of convos will be a msg_wrap

typedef struct{
  char* user; // The user the message came from
  char* content; // The content of the message
}msg;

typedef struct{
  int len;
  void* msgs;
}msg_wrapper;

char* readConvo(char* topic);

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  convos = make_dictionary(COMPARE_CASE_SENS, free);
  
  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  /* Don't kill the server if there's an error, because
     we want to survive errors due to a client. But we
     do want to report errors. */
  exit_on_error(0);

  /* Also, don't stop on broken connections: */
  Signal(SIGPIPE, SIG_IGN);

  appendToConvo("Kylie's hot", "aaron", "Seriously though");
  printf("%s\n", readConvo("Kylie's hot"));
  appendToConvo("Kylie's hot", "aaron", "I like her a lot");
  printf("%s\n", readConvo("Kylie's hot"));
  appendToConvo("Kylie's hot", "aaron", "I like her a lot");
  printf("%s\n", readConvo("Kylie's hot"));
  appendToConvo("Kylie's hot", "aaron", "I like her a lot");
  printf("%s\n", readConvo("Kylie's hot"));
  
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (connfd >= 0) {
      Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                  port, MAXLINE, 0);
      printf("Accepted connection from (%s, %s)\n", hostname, port);
      doit(connfd);
      Close(connfd);
    }
  }
}

/*
 * doit - handle one HTTP request/response transaction
 */
void doit(int fd)
{
    char buf[MAXLINE], *method, *uri, *version;
  rio_t rio;
  dictionary_t *headers, *query;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  printf("%s", buf);
 
  if (!parse_request_line(buf, &method, &uri, &version)) {
    printf("URI: %s\n", uri);
    clienterror(fd, method, "400", "Bad Request",
                "TinyChat did not recognize the request");
  } else {
    if (strcasecmp(version, "HTTP/1.0")
        && strcasecmp(version, "HTTP/1.1")) {
      clienterror(fd, version, "501", "Not Implemented",
                  "TinyChat does not implement that version");
    } else if (strcasecmp(method, "GET")
               && strcasecmp(method, "POST")) {
      clienterror(fd, method, "501", "Not Implemented",
                  "TinyChat does not implement that method");
    } else {
      headers = read_requesthdrs(&rio);

      /* Parse all query arguments into a dictionary */
      query = make_dictionary(COMPARE_CASE_SENS, free);
      parse_uriquery(uri, query);
      if (!strcasecmp(method, "POST")){
	char* content = dictionary_get(query, "content");
	printf("content is %s\n", content);
        read_postquery(&rio, headers, query);
     
      }
      /* For debugging, print the dictionary */
      print_stringdictionary(query);

      if(starts_with("/reply", uri))
	serve_form(fd, query);
      else
	serve_login(fd, query);

      /* Clean up */
      free_dictionary(query);
      free_dictionary(headers);
    }

    /* Clean up status line */
    free(method);
    free(uri);
    free(version);
  }
}

/*
 * read_requesthdrs - read HTTP request headers
 */
dictionary_t *read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];
  dictionary_t *d = make_dictionary(COMPARE_CASE_INSENS, free);

  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    parse_header_line(buf, d);
  }
 
  return d;
}

void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *dest)
{
  char *len_str, *type, *buffer;
  int len;
 
  len_str = dictionary_get(headers, "Content-Length");
  len = (len_str ? atoi(len_str) : 0);

  type = dictionary_get(headers, "Content-Type");
 
  buffer = malloc(len+1);
  Rio_readnb(rp, buffer, len);
  buffer[len] = 0;

  if (!strcasecmp(type, "application/x-www-form-urlencoded")) {
    parse_query(buffer, dest);
  }

  free(buffer);
}

static char *ok_header(size_t len, const char *content_type) {
  char *len_str, *header;
 
  header = append_strings("HTTP/1.0 200 OK\r\n",
                          "Server: TinyChat Web Server\r\n",
                          "Connection: close\r\n",
                          "Content-length: ", len_str = to_string(len), "\r\n",
                          "Content-type: ", content_type, "\r\n\r\n",
                          NULL);
  //free(len_str);

  return header;
}

/*
 * serve_form - sends a form to a client
 */
void serve_form(int fd, dictionary_t* query) 
{
  size_t len;
  char *body, *header;
 
  char* user = dictionary_get(query, "user");
  char* topic = dictionary_get(query, "topic");
  char* content = dictionary_get(query, "content");
  if(user == NULL)
    user = "Grayson";
  if(topic == NULL)
    topic = "Kylie's Hot";

  // Need to get all conversation mesages here
  char* convoText = readConvo(topic);
  printf("%s\n", convoText);
  
  body = append_strings("<html><body>\r\n",
                        "<p>Welcome to TinyChat, ",
			user,
			"</p>",
			"<p>",
			topic,
			"</p>",
			convoText,
                        "\r\n<form action=\"reply\" method=\"post\"",
                        " enctype=\"application/x-www-form-urlencoded\"",
                        " accept-charset=\"UTF-8\">\r\n",
                        "<input type=\"text\" name=\"content\">\r\n",
                        "<input type=\"submit\" value=\"Send\">\r\n",
			"<input type=\"hidden\" name=\"user\" value=\"", user, "\">\r\n",
			"<input type=\"hidden\" name=\"topic\" value=\"", topic, "\">\r\n",
                        "</form></body></html>\r\n",
                        NULL);
 
  len = strlen(body);

  /* Send response headers to client */
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  printf("Response headers:\n");
  printf("%s", header);

  /* printf("Before\n"); */
  /* free(header); // Seg fault here */
  /* printf("After\n"); */

  /* Send response body to client */
  Rio_writen(fd, body, len);

  // Add message to the dictionary
  appendToConvo(topic, user, content);
  
  //free(body);
}

// Adds a message to the conversation specified in topic
// Creates a new entry for topic if it does not exist
// TODO: Need to copy content into a different memory location that won't be freed by doit
void appendToConvo(char* topic, char* user, char* content){
  if(content == NULL || !strcmp(content, "")){
    printf("Returning early\n");
    return;
  }
  char* contentTwo = malloc(strlen(content));
  char* userTwo = malloc(strlen(user));
  strcpy(contentTwo, content);
  strcpy(userTwo, user);
  printf("Content in append: %s\n", contentTwo);
  msg_wrapper* wrap = (msg_wrapper*)dictionary_get(convos, topic);
  if(wrap == NULL){
    printf("Creating a new conversation\n");
    wrap = calloc(1, sizeof(msg_wrapper));
    wrap->len = 0;
    wrap->msgs = NULL;
    dictionary_set(convos, topic, (void*)wrap);
  }

  msg* oldMsgs = (msg*)wrap->msgs;
  msg* newMsgs = (msg*)calloc((wrap->len + 1), sizeof(msg)); // Allocate a new array with one extra message slot

  // Copy the old values over
  int i;
  for(i = 0; i < wrap->len; i++){
    newMsgs[i] = oldMsgs[i];
    printf("%s\n", oldMsgs[i].content);
  }

  msg* newMsg = (msg*)calloc(1, sizeof(msg));
  newMsg->user = userTwo;
  newMsg->content = contentTwo;
  
  newMsgs[wrap->len] = *newMsg;

  //int oldLength = wrap->len;
  wrap->len = wrap->len + 1;
  wrap->msgs = (void*)newMsgs;
  // printf("Print me! %d\n", oldLength);
  //printf("Added new message! User: %s \tContent: %s\n", newMsgs[oldLength].user, newMsg[oldLength].content);
}

// Reads the conversation of the specified topic into an array of messages of length len// Returns NULL if the conversation does not exist
char* readConvo(char* topic){
  printf("Reading convo\n");
  msg_wrapper* wrap = (msg_wrapper*)dictionary_get(convos, topic); 
  if(wrap == NULL)
    return ""; 

  // Figure out how long the string will need to be.
  int len = 0;
  int i;
  for(i = 0; i < wrap->len; i++){
    msg curr = ((msg*)(wrap->msgs))[i];
    len += strlen(curr.user);
    len += strlen(curr.content);
    len += 2;
    len += 6; // Opening and closing paragraph tags
    len += 3; // Newline;
  }

  // Build in the new html
  char* html = calloc(len, sizeof(char));
  for(i = 0; i < wrap->len; i++){
    msg curr = ((msg*)(wrap->msgs))[i];
    html = strcat(html, "<p>\n");
    html = strcat(html, curr.user);
    html = strcat(html, ": ");
    html = strcat(html, curr.content);
    html = strcat(html, "\n</p>\n");
  }

  return html;
}

/*
 * serve_login - sends a form to a client
 */
void serve_login(int fd, dictionary_t* query)
{
  size_t len;
  char *body, *header;
 
  char* user = dictionary_get(query, "user");
  char* topic = dictionary_get(query, "topic");
  if(user == NULL)
    user = "Grayson";
  if(topic == NULL)
    topic = "Kylie's Hot";
 
  body = append_strings("<html><body>\r\n",
                      "<p>Welcome to TinyChat, please log in</p>",
                      "\r\n<form action=\"reply\" method=\"post\"",
                      " enctype=\"application/x-www-form-urlencoded\"",
                      " accept-charset=\"UTF-8\">\r\n",
		      "<input type=\"text\" name=\"user\">\r\n",
		      "<input type=\"text\" name=\"topic\">\r\n",
                      "<input type=\"submit\" value=\"Login\">\r\n",
                      "</form></body></html>\r\n",
                      NULL);
 
  len = strlen(body);

  /* Send response headers to client */
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  printf("Response headers:\n");
  printf("%s", header);

  free(header);

  /* Send response body to client */
  Rio_writen(fd, body, len);

  free(body);
}

/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum,
         char *shortmsg, char *longmsg)
{
  size_t len;
  char *header, *body, *len_str;

  body = append_strings("<html><title>Tiny Error</title>",
                        "<body bgcolor=""ffffff"">\r\n",
                        errnum, " ", shortmsg,
                        "<p>", longmsg, ": ", cause,
                        "<hr><em>The Tiny Web server</em>\r\n",
                        NULL);
  len = strlen(body);

  /* Print the HTTP response */
  header = append_strings("HTTP/1.0 ", errnum, " ", shortmsg,
                          "Content-type: text/html; charset=utf-8\r\n",
                          "Content-length: ", len_str = to_string(len), "\r\n\r\n",
                          NULL);
  free(len_str);
 
  Rio_writen(fd, header, strlen(header));
  Rio_writen(fd, body, len);

  free(header);
  free(body);
}

static void print_stringdictionary(dictionary_t *d)
{
  int i, count;

  count = dictionary_count(d);
  for (i = 0; i < count; i++) {
    printf("%s=%s\n",
           dictionary_key(d, i),
           (const char *)dictionary_value(d, i));
  }
  printf("\n");
}
