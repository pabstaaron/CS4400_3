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

#define SBUF_SIZE 10
#define NUM_THREADS 4

void doit(void* fd);
dictionary_t *read_requesthdrs(rio_t *rp);
void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *d);
void parse_query(const char *uri, dictionary_t *d);
void serve_form(int fd, dictionary_t* query);
void clienterror(int fd, char *cause, char *errnum,
         char *shortmsg, char *longmsg);
static void print_stringdictionary(dictionary_t *d);
void serve_login(int fd, dictionary_t* query);
void appendToConvo(char* topic, char* user, char* content);
void serve_convo(int fd, dictionary_t* query);
void grab_content(int fd, dictionary_t* query);
void serve_import(int fd, dictionary_t* query);
char* readConvo_noHtml(char* topic);
void spin_thread(int fd);
void serve_convo_import(int fd, dictionary_t* query);
void *handle_threads(void *na);
  
dictionary_t* convos; // Each value of convos will be a msg_wrap

typedef struct{
  char* user; // The user the message came from
  char* content; // The content of the message
}msg;

typedef struct{
  int len;
  void* msgs;
}msg_wrapper;

// Data structure used to implemenent a locking mechanism on the convo dict
typedef struct{
  sem_t sem;
  dictionary_t* convos;
}convo_dict;

typedef struct {
  int *buf; /* Buffer array */
  int n; /* Maximum number of slots */
  int front; /* buf[(front+1)%n] is first item */
  int rear; /* buf[rear%n] is last item */
  sem_t mutex; /* Protects accesses to buf */
  sem_t slots; /* Counts available slots */
  sem_t items; /* Counts available items */
} sbuf_t;

char* readConvo(char* topic);
void sbuf_insert(sbuf_t *sp, int item);
void sbuf_init(sbuf_t *sp, int n);
int sbuf_remove(sbuf_t *sp);

//convo_dict* make_convo_dict()

sem_t sema;
sbuf_t connfds;

//convo_dict* convos;

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  
  Sem_init(&sema, 0, 1);
  sbuf_init(&connfds, SBUF_SIZE);
  
  //convos = make_convo_dict();
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

  appendToConvo("Kylie", "aaron", "Seriously though");
  printf("%s\n", readConvo("Kylie"));
  appendToConvo("Kylie hot", "aaron", "I like her a lot");
  printf("%s\n", readConvo("Kylie"));
  appendToConvo("Kylie", "aaron", "I like her a lot");
  printf("%s\n", readConvo("Kylie"));
  appendToConvo("Kylie", "aaron", "I like her a lot");
  printf("%s\n", readConvo("Kylie"));

  // Set up threads
  pthread_t th;
  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    Pthread_create(&th, NULL, handle_threads, NULL);
    Pthread_detach(th);
  }
  
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (connfd >= 0) {
      Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                  port, MAXLINE, 0);
      //printf("Accepted connection from (%s, %s)\n", hostname, port);
      // We'll want to spin off a new thread here instead of calling doit
      //spin_thread(connfd);
      //doit(connfd);
      //Close(connfd);

      sbuf_insert(&connfds, connfd);
    }
  }
}

convo_dict* make_convo_dict(){
  convo_dict* cd = malloc(sizeof(convo_dict));
  cd->convos = make_dictionary(COMPARE_CASE_SENS, free);
  Sem_init(&cd->sem, 0, 1);
  return cd;
}

// Locks up the dictionary and returns the desired conversation
msg_wrapper* get_msg_wrapper(convo_dict* cd, char* topic){
  P(&cd->sem);
  msg_wrapper* wrap = (msg_wrapper*)dictionary_get(cd->convos, topic);
}

void unlock_dict(convo_dict* cd){
  V(&cd->sem);
}

void sbuf_init(sbuf_t *sp, int n) {
  sp->buf = Calloc(n, sizeof(int));
  sp->n = n; /* max of n items */
  sp->front = sp->rear = 0; /* empty iff front == rear */
  Sem_init(&sp->mutex, 0, 1); /* for locking */
  Sem_init(&sp->slots, 0, n); /* initially n empty slots */
  Sem_init(&sp->items, 0, 0); /* initially zero data items */
}

void sbuf_insert(sbuf_t *sp, int item) {
  P(&sp->slots); /* wait for available slot */
  P(&sp->mutex); /* lock */
  sp->buf[(++sp->rear)%(sp->n)] = item;
  V(&sp->mutex); /* unlock */
  V(&sp->items); /* announce available item */
}

int sbuf_remove(sbuf_t *sp) {
  int item;
  P(&sp->items); /* wait for available item */
  P(&sp->mutex); /* lock */
  item = sp->buf[(++sp->front)%(sp->n)];
  V(&sp->mutex); /* unlock */
  V(&sp->slots); /* announce available slot */
  return item;
}

void spin_thread(int fd){
  int* fd_p = malloc(sizeof(int));
  *fd_p = fd;

  pthread_t th;
  Pthread_create(&th, NULL, doit, fd_p); // Create a thread running doit
  Pthread_detach(th); // We don't care about the result, so we don't need to wait on the thread
}

void *handle_threads(void *na){
  int connfd;
  while(1){
    connfd = sbuf_remove(&connfds);

    doit((void*) &connfd);
  }

  return NULL;
}

/*
 * doit - handle one HTTP request/response transaction
 */
void doit(void* fd_p)
{
  int fd = *((int*)fd_p);
  
  char buf[MAXLINE], *method, *uri, *version;
  rio_t rio;
  dictionary_t *headers, *query;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  //printf("%s", buf);
 
  if (!parse_request_line(buf, &method, &uri, &version)) {
    // printf("URI: %s\n", uri);
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
	//printf("content is %s\n", content);
        read_postquery(&rio, headers, query);
      }
      /* For debugging, print the dictionary */
      //print_stringdictionary(query);

      
      // Will need to add some additional logic here to service
      if(starts_with("/conversationimport", uri)){
	// Send the conversation specified in the url as plain
	serve_convo_import(fd, query);
      }
      else if(starts_with("/conversation", uri)){
	// Send the conversation specified in the url as plain
	serve_convo(fd, query);
      }
      else if(starts_with("/say", uri)){
	// Add the content specified in the query into the topic specified
	grab_content(fd, query);
      }
      else if(starts_with("/import", uri)){
	// Imports a conversation from another chat server
	serve_import(fd, query);
      }
      else if(starts_with("/reply", uri))
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
  Close(fd);
  //return NULL;
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
  free(len_str);

  return header;
}

// Takes a message line containing the username and message content
// and returns a string array containing the username and message content
char** extractMessage(char* msg){
  //printf("Unextracted msg: %s\n", msg);
  
  char* msg2 = strdup(msg);

  int tokens_alloc = 1;
  int tokens_used = 0;

  char** tokens = calloc(tokens_alloc, sizeof(char*));
  char* token;
  char* strtok_cntx;
  char* delim = ":"; // string we want to split around
  
  for(token = strtok_r(msg, ": ", &strtok_cntx); token != NULL; token = strtok_r(NULL, delim, &strtok_cntx)){
    if(tokens_used == tokens_alloc){
      tokens_alloc *= 2;
      tokens = realloc(tokens, tokens_alloc * sizeof(char*));
    }
    tokens[tokens_used++] = strdup(token);
  }

  if(tokens_used == 0){
    free(tokens);
    tokens = NULL;
  }
  else
    tokens = realloc(tokens, tokens_used * sizeof(char*));

  free(msg2);

  // At this point, there will be an extra space at the beginning of content
  int origlen = strlen(tokens[1]);
  char* content2 = malloc(origlen); // seg fault occuring here
  int i;
  for(i = 0; i < origlen-1; i++){
    content2[i] = tokens[1][i+1];
  }
  content2[origlen-1] = '\0';
  tokens[1] = content2;
  
  return tokens;
}

/*
 * Imports a conversation from another chat server
 * TODO - Picking up an extra newline somewhere
 */
void serve_import(int fd, dictionary_t* query){
  // Need to fire up a connection to another server, make a conversation request
  // And then add that data into our own conversation data structure

  // Pull out needed information
  char* topic = dictionary_get(query, "topic");
  char* host = dictionary_get(query, "host");
  char* port = dictionary_get(query, "port");
    
  int cFd = Open_clientfd(host, port);
  
  char* req = append_strings("GET /conversationimport?topic=",
			     topic,
			     " HTTP/1.0\r\n\r\n",
			     NULL);

  // Send HTTP request
  Rio_writen(cFd, req, strlen(req)); // Will this send the right URL?

  // Need to read incoming transmission line by line
  rio_t rio;
  char buf[MAXLINE];
  int len;
  rio_readinitb(&rio, cFd);
  while((len = rio_readlineb(&rio, buf, MAXLINE)) != 0){
    // Need to ignore HTTP 
    //printf("%s", buf);
    char** msg = extractMessage(buf);
    //printf("-Extracted message-\n");
    //printf("user: %s\n", msg[0]); 
    //printf("content: %s\n", msg[1]);
    appendToConvo(topic, strdup(msg[0]), strdup(msg[1]));
    free(msg[0]);
    free(msg[1]);
    free(msg);
  }
  
  char* header = ok_header(0, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  free(header); 
}

/* 
 * Adds the content specified in query to the conversation specified in query
 */
void grab_content(int fd, dictionary_t* query){
  char* topic = dictionary_get(query, "topic");
  char* user = dictionary_get(query, "user");
  char* content = dictionary_get(query, "content"); 

  appendToConvo(topic, user, content);

  char* header = ok_header(0, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  free(header); 
}

/*
 * Sends the content of the of the conversation specified
 * in the query's topic field as plain text to the client
 */
void serve_convo(int fd, dictionary_t* query){
  // Step 1: Get conversation data
  //printf("In regular version\n");
  
  char* topic = dictionary_get(query, "topic");
  char* convoText = readConvo_noHtml(topic);

  //printf("Sending conversation: %s\nContent: %s\n", topic, convoText);
  
  // Step 2: Send that data back to the client
  // TODO - This might need to be an HTTP response

  char* header = ok_header(strlen(convoText), "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  free(header);
  
  Rio_writen(fd, convoText, strlen(convoText));

  if(strlen(convoText) != 0 && convoText != NULL)
    free(convoText);
}

void serve_convo_import(int fd, dictionary_t* query){
  // Step 1: Get conversation data
  //printf("In import version\n");
  char* topic = dictionary_get(query, "topic");
  char* convoText = readConvo_noHtml(topic);

  //printf("Sending conversation: %s\nContent: %s\n", topic, convoText);
  
  // Step 2: Send that data back to the client
  // TODO - This might need to be an HTTP response

  /* char* header = ok_header(strlen(convoText), "text/html; charset=utf-8"); */
  /* Rio_writen(fd, header, strlen(header)); */
  /* free(header); */
  
  Rio_writen(fd, convoText, strlen(convoText));

  //free(convoText);
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
   // Add message to the dictionary
  appendToConvo(topic, user, content);
  char* convoText = readConvo(topic);
  //printf("%s\n", convoText);
  
  body = append_strings("<html><body>\r\n",
                        "<p>Welcome to TinyChat, ",
			user,
			"</p>",
			"<p>Topic: ",
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
  //printf("Response headers:\n");
  //printf("%s", header);

  /* printf("Before\n"); */
  free(header); 
  /* printf("After\n"); */

  /* Send response body to client */
  Rio_writen(fd, body, len);

 
  free(convoText);
  free(body);
}

// Adds a message to the conversation specified in topic
// Creates a new entry for topic if it does not exist
// TODO: Need to copy content into a different memory location that won't be freed by doit
void appendToConvo(char* topic, char* user, char* content){
  if(content == NULL || !strcmp(content, "")){
    //printf("Returning early\n");
    return;
  }
  char* contentTwo = malloc((strlen(content)+1) * sizeof(char));
  char* userTwo = malloc((strlen(user)+1) * sizeof(char));
  strcpy(contentTwo, content);
  strcpy(userTwo, user);
  //printf("Content in append: %s\n", contentTwo);
  P(&sema); // Install a lock
  msg_wrapper* wrap = (msg_wrapper*)dictionary_get(convos, topic);
  if(wrap == NULL){ 
    //printf("Creating a new conversation\n");
    //P(&sema); // Install a lock
    wrap = calloc(1, sizeof(msg_wrapper));
    wrap->len = 0;
    wrap->msgs = NULL;
    dictionary_set(convos, topic, (void*)wrap);
    //V(&sema); // Unlock
  }

  msg* oldMsgs = (msg*)wrap->msgs;
  msg* newMsgs = (msg*)calloc((wrap->len + 1), sizeof(msg)); // Allocate a new array with one extra message slot

  // Copy the old values over
  int i;
  for(i = 0; i < wrap->len; i++){
    newMsgs[i] = oldMsgs[i];
    //printf("%s\n", oldMsgs[i].content);
  }

  msg* newMsg = (msg*)calloc(1, sizeof(msg));
  newMsg->user = userTwo;
  newMsg->content = contentTwo; 
  
  newMsgs[wrap->len] = *newMsg;

  //int oldLength = wrap->len;
  //P(&sema); // Install a lock
  wrap->len = wrap->len + 1;
  wrap->msgs = (void*)newMsgs; 
  //V(&sema); // Unlock
  // printf("Print me! %d\n", oldLength);
  //printf("Added new message! User: %s \tContent: %s\n", newMsgs[oldLength].user, newMsg[oldLength].content);
  //printf("Bailing out of append\n");

  free(oldMsgs);
  V(&sema); // Unlock
  // End Critical Region
}

// Reads the conversation of the specified topic into an array of messages of length len// Returns NULL if the conversation does not exist
char* readConvo(char* topic){
  //printf("Reading convo\n");
  //P(&sema);
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
  char* html = calloc(len+1, sizeof(char));
  for(i = 0; i < wrap->len; i++){
    msg curr = ((msg*)(wrap->msgs))[i];
    html = strcat(html, "<p>");
    html = strcat(html, curr.user);
    html = strcat(html, ": ");
    html = strcat(html, curr.content);
    html = strcat(html, "</p>");
  }

  //V(&sema);
  return html;
}

char* readConvo_noHtml(char* topic){
  //printf("Reading convo\n");
  //P(&sema);
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
    len += 4; 
  }

  // Build in the new html
  char* html = calloc(len + 4, sizeof(char));
  for(i = 0; i < wrap->len; i++){
    msg curr = ((msg*)(wrap->msgs))[i];
    html = strcat(html, curr.user);
    html = strcat(html, ": ");
    html = strcat(html, curr.content);
    html = strcat(html, "\r\n"); // Carriage return
  }
  //V(&sema);
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
  //printf("Response headers:\n");
  //printf("%s", header);

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
