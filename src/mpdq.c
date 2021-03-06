/**
 * MPDQ
 * Music Player Daemon Query
 * Adding trayicons to control mpd
 * Licenced under MPL 2.0
 */
#include <X11/Xlib.h>
#include <mpd/client.h>
#include <mpd/stats.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <src/util.h>
#include <src/cfg.h>

#define STATE_UNKOWN 0
#define STATE_STOPPED 1
#define STATE_WORK 2

#define BTN_PLAY 1
#define MPD_PLAYING 2
#define MPD_PAUSED 3

/*　Vars */
static volatile     int runFlag = 1;
static struct       mpd_connection   *conn;
static struct       mpd_status       *status;
static struct       mpd_song         *song;
static enum         mpd_state        state = 0;
static char         *song_name;
static char         *artist_name;
static char         *dwm_title;
static char         *cmd;
static char         *base_cmd = "xsetroot -name \"";
static char         *cmd_end  = "\"";
static char         *space    = " - ";
static bool         no_refresh = 0;
static Config       *cfg;

/* Util Methods*/
void stop(int d) { runFlag = 0; }
void clean_up(void);

int main(int argc, char const *argv[])
{
    // Listen for CTRL + C
    signal(SIGINT, stop);

    cfg = create_or_open_cfg("./mpdq.cfg");

    // Connect to local MPD
    conn = mpd_connection_new(NULL, 0, 0);
    if (conn)
        _log("Connected!\n");
    else if (!conn)
        die("Connection failure! No local MPD session running!\n", 1);

    initX(cfg);

    // Vars
    int event_result = -1;
    debug = argc > 1;

    char* sub_dwm_title[cfg->max_song_length];
    char* set_root_cmd = append("xsetroot -name ", cfg->title_text);

    // Get song
    while (runFlag)
    {
        status = mpd_run_status(conn);
        song   = mpd_run_current_song(conn);
        state  = mpd_status_get_state(status);

        if (!song)
        {
            _log("Couldn't recieve current song!\n");
            system(set_root_cmd);
        }
        else if (!status)
        {
            _log("Couldn't recieve status of MPD!\n");
            system(set_root_cmd);
        }
        else
        {
             // Get song info
            song_name = (char *) mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
            artist_name = (char *) mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

            if (artist_name)
            {
                dwm_title = append(append(artist_name, space), song_name);
            }
            else
            {
                dwm_title = song_name;
            }

            if (cfg->song_to_text_file && dwm_title != NULL)
                system(append(append(append("echo \"", dwm_title), "\" > "), cfg->file_path)); // NESTING FTW

            if (cfg->song_to_root_window && song_name)
            {
                 switch (state)
                {
                    case STATE_UNKOWN:
                    case STATE_STOPPED:
                            system(set_root_cmd); // Stopped/Unknown status
                            no_refresh = 1;
                        break;
                    case STATE_WORK:
                            if (strlen(dwm_title) > cfg->max_song_length + 3) // Cut off songs
                            {
                                memcpy(sub_dwm_title, &dwm_title[0], cfg->max_song_length - 4);

                                if (volume != NULL)
                                {
                                    cmd = append(sub_dwm_title, volume);
                                    cmd = append(base_cmd, cmd);
                                    cmd = append(cmd, cmd_end);
                                }
                                else
                                {
                                    cmd = append(base_cmd, sub_dwm_title);
                                    cmd = append(cmd, cmd_end);
                                }
                            }
                            else
                            {
                                if (volume != NULL)
                                    dwm_title = append(dwm_title, volume);
                                cmd = append(base_cmd, dwm_title);
                                cmd = append(cmd, cmd_end);
                            }
                            no_refresh = 0;
                }               
            }

            if (cfg->song_to_root_window && !no_refresh && cmd != NULL) // No need to set the same name twice
                system(cmd);

            if (volume != NULL)
            {
                if (cycles * cfg->delay >= cfg->volume_timeout)
                {
                    cycles = 0;
                    volume = NULL;
                }
                cycles++;
            }
            else
            {
                cycles = 0;
            }
        }

        if (cfg->enable_hotkeys || cfg->enable_icons)
            event_result = handle_events();

        if (event_result == BTN_PLAY) // Toggle state playing <--> paused, without waiting for the next update cycle
            state = state == MPD_PLAYING ? MPD_PAUSED : MPD_PLAYING;

        if (cfg->enable_icons)
            draw_tray(state, cfg->icon_color);

        if (event_result > -1)
            tray_window_event(event_result, state, status, conn);

        usleep(1000 * cfg->delay); // 500ms update cycle
    }

    printf("Disconnecting...\n");
    
    if (cfg->song_to_root_window);
        system(set_root_cmd);

    // Close Connection & free resources
    clean_up();
    return 0;
}

void clean_up(void)
{
    if (cfg->enable_icons)
        destroy_tray_icons();
    
    if (cfg->enable_hotkeys)
        clear_keybinds(cfg);
    
    mpd_status_free(status);
    mpd_connection_free(conn);
    mpd_song_free(song);
    
    song_name = NULL;
    artist_name = NULL;
    space = NULL;
    cmd = NULL;
    base_cmd = NULL;
    cmd_end = NULL;
    free(cfg);
}
