#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <regex.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <assert.h>
#include <dirent.h>

#include "/bxr_plover/src/rabbitmq-c/examples/utils.h"
#include "/bxr_plover/src/rabbitmq-c/examples/utils.c"
#include "/bxr_plover/src/glade/b64.h"
#include "/bxr_plover/src/glade/encode.c"
#include "/bxr_plover/src/glade/decode.c"

#define MAX_ERROR_MSG 0x1000
#define MAX_CNT 10

typedef struct Data_storage
{
    char	fpath[300];	//파일 경로
    char	fname[20];		//파일 이름
    int	cnt;			//민감정보 총 개수
    int	fsize;			//파일 크기
	char	stat;			//파일 상태
	
	
}data_storage;

data_storage ds_j[MAX_CNT]; //j: 주민번호, d: 운전면허

static gchar *path;
int chk_tf; //chk_true false
int data_flag = 1; //어떤종류의 민감정보인지 확인하기위한 flag



GtkWidget	*detect_window,
			*setting_window;
			
GtkEntry	*d_detect_entry;

void e_enroll_btn_clicked	 	(GtkButton *e_enroll_btn,	gpointer *data);

void m_window_destroy();
void m_detect_btn_clicked		(GtkButton *m_detect_btn,	gpointer *data);
void m_setting_btn_clicked	(GtkButton *m_setting_btn,	gpointer *data);

void d_detect_btn_clicked		(GtkButton *d_detect_btn,	gpointer *data);
void d_option_btn_clicked		(GtkButton *d_option_btn,	gpointer *data);
void d_folder_btn_clicked		(GtkButton *d_folder_btn,	gpointer *data);
void d_close_btn_clicked		(GtkButton *d_close_btn,	gpointer *data);
void d_detect_entry_activate	(GtkEntry	*d_detect_entry,	gpointer *data);

void s_cloese_btn_clicked		(GtkButton *s_cloese_btn,	gpointer *data);

//Base64 encoding//
char *b64_encode(const unsigned char *src, size_t len, char *enc);


//Compile the regular expression described by "regex_text" into "r"//
int compile_regex (regex_t *r, const char *regex_text)
{
    int status = regcomp(r, regex_text, REG_EXTENDED|REG_NEWLINE);

    if (status != 0)
    {
    char error_message[MAX_ERROR_MSG];

    regerror(status, r, error_message, MAX_ERROR_MSG);

    printf("Regex error compiling '%s': %s\n", regex_text, error_message);

    return 1;
    }

    return 0;
}

//Match the string in "to_match" against the compiled regular expression in "r"//
char match_regex_j (regex_t *r, const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char *p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 100;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    //버퍼크기만큼 읽은 부분 전체를 해당 정규식과 비교//
    while (1)
    {
        int nomatch = regexec(r, p, n_matches, m, 0);

        if (nomatch)
        {
            printf("No more matches.\n");
            return 0;
        }

        else
        {
            for (int i = 0; i < n_matches; i++)
            {
                int start;

                if (m[i].rm_so == -1)
                {
                    break;
                }

                start = m[i].rm_so + (p - to_match);

                //주민번호 정규식 검사 통과//
                if (i == 0)
                {
                    int chk = 0, tmp = 0, sum = 0;
                    char buf_jumin[15] = {0,};
                    //주민번호 유효성 검사//
                    for (int j = 0; j < 14; j++)
                    {
                        buf_jumin[j] = *(to_match + start + j);
                        buf_jumin[j] -= 48;
                    }
                    
                    sum = buf_jumin[0]*2 + buf_jumin[1]*3 + buf_jumin[2]*4 + buf_jumin[3]*5 + buf_jumin[4]*6 + buf_jumin[5]*7
						 + buf_jumin[7]*8 + buf_jumin[8]*9 + buf_jumin[9]*2 + buf_jumin[10]*3 + buf_jumin[11]*4 + buf_jumin[12]*5;

                    chk = buf_jumin[13];
                    tmp = 11 - (sum % 11);

                    if (tmp >= 10)
                    {
                        tmp -= 10;
                    }

                    //주민번호 유효성 통과//
                    if (tmp == chk)
                    {
						ds_j->cnt++; //검출된 주민등록번호의 수//          
                        //주민번호 구조체에 저장//
                        strcpy(ds_j[ds_j->cnt].fpath, filepath);
                        strcpy(ds_j[ds_j->cnt].fname, file->d_name);
                        ds_j[ds_j->cnt].fsize = buf.st_size;

                        printf("cnt: %d, file_path: %s, file_name: %s, file_size: %dbyte\n", ds_j->cnt, ds_j[ds_j->cnt].fpath, ds_j[ds_j->cnt].fname, ds_j[ds_j->cnt].fsize);
                    }
                }
            }
        }

        p += m[0].rm_eo;
    }
}

//Check kind of data//
void check_kind_of_data (const char *to_match, char *filepath, struct dirent *file, struct stat buf)
{
	regex_t r;
	const char *regex_text;

	switch(data_flag)
	{
		case 1:
			regex_text = "([0-9]{2}(0[1-9]|1[0-2])(0[1-9]|[1,2][0-9]|3[0,1])-[1-4][0-9]{6})"; //주민번호 정규식//
			compile_regex(&r, regex_text); //정규식 컴파일//
			match_regex_j(&r, to_match, filepath, file, buf);
			
			break;
			
		case 2:
			regex_text = "[0-9]{2}[-~.[:space:]][0-9]{6}[-~.[:space:]][0-9]{2}"; //운전면허 정규식//
			compile_regex(&r, regex_text); //정규식 컴파일//
			//match_regex_j(&r, to_match, filepath, file, buf);
	}
}

int scan_dir (gchar *path)
{
    DIR *dp = NULL;
    FILE *fp = NULL;
    struct dirent *file = NULL;
    struct stat buf;
    char filepath[300];
    char buffer[5000];
    const char *find_text;

    if ((dp = opendir(path)) == NULL)
    {
        printf("폴더를 열수 없습니다.\n");

        return -1;
    }

    while ((file = readdir(dp)) != NULL)
    {
        //filepath에 현재 path넣기//
        sprintf(filepath, "%s/%s", path, file->d_name);
        lstat(filepath, &buf);

        //폴더//
        if (S_ISDIR(buf.st_mode))
        {
            // .이거하고 ..이거 제외//
            if ((!strcmp(file->d_name, ".")) || (!strcmp(file->d_name, "..")))
            {
                continue;
            }

            //안에 폴더로 재귀함수//
            scan_dir(filepath);
        }

        //파일//
        else if (S_ISREG(buf.st_mode))
        {
            fp = fopen(filepath, "r");

            if (NULL == fp)
            {
                printf("파일을 열수 없습니다.\n");
                return 1;
            }



            //버퍼 크기만큼 읽고 find_text에 넣어서 정규식검사로 이동//
            while (feof(fp) == 0)
            {
                fread(buffer, sizeof(char), sizeof(buffer), fp);
                find_text = buffer;
                check_kind_of_data(find_text, filepath, file, buf);
                find_text = NULL;
            }

            //메모리관리(초기화), 파일닫기//
            memset(buffer, 0, sizeof(buffer));
            fclose(fp);
        }
    }
    closedir(dp);
    
    return  0;
}

int detect_func(gchar *path)
{
	char *hostname;
	int port, status;
	char *exchange;
	char *routingkey;
	static char *enc;
	
	amqp_socket_t *socket = NULL;
	amqp_connection_state_t conn;
	amqp_bytes_t reply_to_queue;

	hostname = "127.0.0.1";
	port = atoi("5672");
	exchange = "aa";
	routingkey = "ka";

	scan_dir(path);
	
	/*
	 establish a channel that is used to connect RabbitMQ server
	*/
	conn = amqp_new_connection();

	socket = amqp_tcp_socket_new(conn);
	
	if (!socket)
	{
	die("creating TCP socket");
	}

	status = amqp_socket_open(socket, hostname, port);
	
	if (status)
	{
	die("opening TCP socket");
	}


	die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
																		"guest", "guest"),
																		"Logging in");

					
	amqp_channel_open(conn, 1);

	die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");


	/*
	 create private reply_to queue
	*/
	{
		amqp_queue_declare_ok_t *r = amqp_queue_declare(
			conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
			
			
		die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");

		
		reply_to_queue = amqp_bytes_malloc_dup(r->queue);
		if (reply_to_queue.bytes == NULL)
		{
			fprintf(stderr, "Out of memory while copying queue name");
			return 1;
		}
	}

	/*
	 send the message
	*/
	{
		/*
		  set properties
		*/
		amqp_basic_properties_t props;
		props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG |
					   AMQP_BASIC_DELIVERY_MODE_FLAG | AMQP_BASIC_REPLY_TO_FLAG |
					   AMQP_BASIC_CORRELATION_ID_FLAG;
		props.content_type = amqp_cstring_bytes("text/plain");
		props.delivery_mode = 2; /* persistent delivery mode */
		props.reply_to = amqp_bytes_malloc_dup(reply_to_queue);
		
		if (props.reply_to.bytes == NULL)
		{
			fprintf(stderr, "Out of memory while copying queue name");
			return 1;
		}
		props.correlation_id = amqp_cstring_bytes("1");

		/*
		  publish
		*/
		for(int i = 1; i <= ds_j->cnt; i++)
		{
			size_t in_len = sizeof(ds_j[i]);
			//printf("ds_j[%d]: %ld\n", i ,in_len); //구조체 크기확인

			
			enc = b64_encode((unsigned char *)&ds_j[i], in_len, enc);
			printf("enc_data: %s\n", enc);
			printf("cnt: %d, file_path: %s, file_name: %s, file_size: %dbyte\n\n", i, ds_j[i].fpath, ds_j[i].fname, ds_j[i].fsize);
			
			die_on_error(amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange),
													amqp_cstring_bytes(routingkey), 0, 0,
													&props, amqp_cstring_bytes(enc)), "Publishing");

			
		}
		amqp_bytes_free(props.reply_to);
	}

	/*
	wait an answer
	*/
	{
		amqp_basic_consume(conn, 1, reply_to_queue, amqp_empty_bytes, 0, 1, 0,
								amqp_empty_table);
		die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");


		{
			amqp_frame_t frame;
			int result;

			amqp_basic_deliver_t *d;
			amqp_basic_properties_t *p;
			size_t body_target;
			size_t body_received;

			for (;;)
			{
				amqp_maybe_release_buffers(conn);
				result = amqp_simple_wait_frame(conn, &frame);
				printf("Result: %d\n", result);
			
				if (result < 0)
				{
					break;
				}

				printf("Frame type: %u channel: %u\n", frame.frame_type, frame.channel);
				if (frame.frame_type != AMQP_FRAME_METHOD)
				{
					continue;
				}

				printf("Method: %s\n", amqp_method_name(frame.payload.method.id));
				if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD)
				{
					continue;
				}

				d = (amqp_basic_deliver_t *)frame.payload.method.decoded;
				printf("Delivery: %u exchange: %.*s routingkey: %.*s\n",
					   (unsigned)d->delivery_tag, (int)d->exchange.len,
					   (char *)d->exchange.bytes, (int)d->routing_key.len,
					   (char *)d->routing_key.bytes);

				result = amqp_simple_wait_frame(conn, &frame);
				if (result < 0)
				{
					break;
				}

				if (frame.frame_type != AMQP_FRAME_HEADER)
				{
					fprintf(stderr, "Expected header!");
					abort();
				}
				p = (amqp_basic_properties_t *)frame.payload.properties.decoded;
				if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG)
				{
					printf("Content-type: %.*s\n", (int)p->content_type.len,
						 (char *)p->content_type.bytes);
				}
				printf("----\n");

				body_target = (size_t)frame.payload.properties.body_size;
				body_received = 0;

				while (body_received < body_target)
				{
					result = amqp_simple_wait_frame(conn, &frame);
					
					if (result < 0)
					{
						break;
					}

					if (frame.frame_type != AMQP_FRAME_BODY)
					{
						fprintf(stderr, "Expected body!");
						abort();
					}

					body_received += frame.payload.body_fragment.len;
					assert(body_received <= body_target);

					amqp_dump(frame.payload.body_fragment.bytes,
								frame.payload.body_fragment.len);
				}

				if (body_received != body_target)
				{
					/* Can only happen when amqp_simple_wait_frame returns <= 0 */
					/* We break here to close the connection */
					break;
				}

				/* everything was fine, we can quit now because we received the reply */
				break;
			}
		}
	}

	/*
	 closing
	*/
	die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS),
															"Closing channel");
															
	die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS),
															"Closing connection");
															
	die_on_error(amqp_destroy_connection(conn), "Ending connection");

	return TRUE;
}

//계정이 있는지 확인: TRUE(1)=있다 FALSE(0)=없다//
int chk_user()
{
	chk_tf = TRUE;
	
	return chk_tf;
}


//main_window_start//
void m_detect_btn_clicked (GtkButton *m_detect_btn, gpointer *data)
{
	gtk_widget_show(detect_window);
	
	return;
}

void m_setting_btn_clicked (GtkButton *m_setting_btn, gpointer *data)
{
	gtk_widget_show(setting_window);
	
	return;
}

void main_window_destroy()
{
    gtk_main_quit();
    
    return;
}
//main_window_end//



//detect_window_start//
void d_detect_entry_activate (GtkEntry *d_detect_entry, gpointer *data)
{
	path = (gchar *)gtk_entry_get_text(d_detect_entry);
	g_print("선택한 폴더 위치: %s\n", path);
	
	return;
}


void d_folder_btn_clicked (GtkButton *d_folder_btn, gpointer *data)
{
	GtkWidget *d_filechooserdialog;
	
    d_filechooserdialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(data), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, 
			("_선택"), GTK_RESPONSE_ACCEPT, NULL);

    gtk_widget_show_all(d_filechooserdialog);
    
	gint resp = gtk_dialog_run(GTK_DIALOG(d_filechooserdialog));

    if( resp == GTK_RESPONSE_ACCEPT)
	{
		gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d_filechooserdialog));
   	} 
   	
	path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d_filechooserdialog));
	
	gtk_entry_set_text(GTK_ENTRY (data), path);

	
	gtk_widget_destroy(d_filechooserdialog);

	g_print("선택한 폴더 위치: %s\n", path);

	return;
}

void d_detect_btn_clicked (GtkButton *d_detect_btn, gpointer *data)
{
	detect_func(path);
	
	return;
}

void d_option_btn_clicked (GtkButton *d_option_btn, gpointer *data)
{
	gtk_widget_show(setting_window);
	
	return;
}

void d_encrypt_btn_clicked (GtkButton *d_encrypt_btn, gpointer *data)//미구현//
{
	return;
}

void d_delete_btn_clicked (GtkButton *d_delete_btn, gpointer *data)//미구현//
{
	return;
}

void d_close_btn_clicked (GtkButton *d_close_btn, gpointer *data)
{
	gtk_widget_hide(GTK_WIDGET(data));
	
	return;
}

void detect_window_destroy (GtkWidget *detect_window, gpointer *data)
{
    gtk_widget_destroy(GTK_WIDGET(detect_window));
    
    return;
}
//detect_window_end//



//enrollment_window_start//
void e_enroll_btn_clicked (GtkButton *e_enroll_btn, gpointer *data)
{
	gtk_widget_hide(GTK_WIDGET(data));
	
	return;
}
//enrollment_window_end//



//setting_setting_start//
void s_cloese_btn_clicked (GtkButton *setting_window, gpointer *data)
{
	gtk_widget_hide(GTK_WIDGET(data));
	
	return;
}
//setting_setting_end//



//main//
int main (int argc, char *argv[])
{
    GtkBuilder	*builder;
    GtkWidget		*main_window,
					*enrollment_window;
					
    gtk_init(&argc, &argv);
	
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "main.glade", NULL);

    main_window				= GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    enrollment_window		= GTK_WIDGET(gtk_builder_get_object(builder, "enrollment_window"));
    detect_window			= GTK_WIDGET(gtk_builder_get_object(builder, "detect_window"));
    setting_window			= GTK_WIDGET(gtk_builder_get_object(builder, "setting_window"));

    
    //닫기x 버튼을 hide로 바꾸기//
    g_signal_connect(detect_window, "delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
    g_signal_connect(setting_window, "delete_event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);

    gtk_builder_connect_signals(builder, NULL);
    
    g_object_unref(builder);

	chk_user(chk_tf);
	
	if (chk_tf == FALSE) //TRUE(1)=있다
	{
		gtk_widget_show(enrollment_window);
		gtk_main();
	}
	
	if (chk_tf == TRUE) 	//FALSE(0)=없다
	{
		gtk_widget_show(main_window);
		gtk_main();
	}

    return 0;
}
