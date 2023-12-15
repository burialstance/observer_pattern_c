#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


enum AppEventTypeEnum {
    AppStartupEventType,
    AppShutdownEventType,
};

typedef void (*listener_callback)();
struct AppEventListener {
    listener_callback callback;
};

typedef struct {
    enum AppEventTypeEnum type;
    const char *name;

    struct AppEventListener *listeners;
    int listeners_count;
} AppEvent;


AppEvent create_app_event(const char *name, enum AppEventTypeEnum type){
    AppEvent event = {
        .name=name,
        .type=type,
        .listeners=NULL,
        .listeners_count=0
    };
    return event;
}

void free_app_event(AppEvent *event){
    free(event->listeners);
}

void emit_event(AppEvent *event){
    printf("emit AppEvent(name=\"%s\")\n", event->name);
    for (int i = 0; i < event->listeners_count; i++) {
        struct AppEventListener listener = event->listeners[i];
        listener.callback();
    }
}

void add_listener_to_event(AppEvent *event, listener_callback listener){
    event->listeners = (struct AppEventListener *)realloc(
        event->listeners,
        sizeof(struct AppEventListener) * (event->listeners_count + 1)
    );
    struct AppEventListener app_event_listener = { listener };
    event->listeners[event->listeners_count] = app_event_listener;
    event->listeners_count++;
}



enum AppState {
    APP_STARTED,
    APP_STOPPED
};

typedef struct {
    const char *title;
    enum AppState state;
    AppEvent on_startup;
    AppEvent on_shutdown;
} App;


App create_app(const char *title){
    App app = {};
    app.title = title;
    app.state = APP_STOPPED;
    app.on_startup = create_app_event("startup", AppStartupEventType);
    app.on_shutdown = create_app_event("shutdown", AppShutdownEventType);

    printf("created app %s\n", app.title);
    return app;
}

void free_app(App *app){
    free_app_event(&app->on_startup);
    free_app_event(&app->on_shutdown);
}

void start_app(App *app){
    printf("call start_app(title=\"%s\")\n", app->title);
    app->state = APP_STARTED;
    emit_event(&app->on_startup);
}

void shutdown_app(App *app){
    printf("call shutdown_app(title=\"%s\")\n", app->title);

    emit_event(&app->on_shutdown);

    app->state = APP_STOPPED;
    free_app(app);
}


// custom
int _random_between(int min, int max) {return rand() % (max - min + 1) + min; }
void _delay(){
    int min = 600000;
    int max = 1200000;
    usleep(_random_between(min, max));
}

void on_startup_connect_to_database(){
    printf("[on_startup] connect_to_database\n");
    _delay();
}

void on_startup_connect_rmq(){
    printf("[on_startup] connect_rmq\n");
    _delay();
}
void on_startup_warmup_cache(){
    printf("[on_startup] warmup_cache\n");
    _delay();
}
void on_shutdown_close_database(){
    printf("[on_shutdown] close_database\n");
    _delay();
}
void notify_admins(){
    printf("[event] notify_admins\n");
    _delay();
}

void connect_lifecycle_handlers(App *app){
    add_listener_to_event(&app->on_startup, on_startup_connect_to_database);
    add_listener_to_event(&app->on_startup, on_startup_connect_rmq);
    add_listener_to_event(&app->on_startup, on_startup_warmup_cache);
    add_listener_to_event(&app->on_startup, notify_admins);

    add_listener_to_event(&app->on_shutdown, on_shutdown_close_database);
    add_listener_to_event(&app->on_shutdown, notify_admins);
}


int main(){
    App app = create_app("Test observer");
    connect_lifecycle_handlers(&app);
    start_app(&app);

    srand(time(0));
    char *stages[] = {
        "\trequest to cache",
        "\tcache expired",
        "\trequest to pgsql",
        "\tset values to cache",
        "\tsend to rmq results",
        NULL
    };
    char **current_stage = stages;
    while (*current_stage) {
        printf("%s\n", *current_stage++);
    }

    shutdown_app(&app);
    return 0;
}
