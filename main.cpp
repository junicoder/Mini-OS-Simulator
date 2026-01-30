
/**
 * main.cpp
 * Entry point. Initializes threads and provides the CLI Menu.
 * OS SIMULATOR BY JUNAID F2023266884
 */

#include "os_sim.h"
#include <thread>
#include <chrono>

// Define Shared Variables
vector<int> available_resources;
queue<Process> buffer;
vector<Process> ready_queue;
vector<Process> blocked_queue;
sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t mutex_lock;
bool simulation_running = true;

// Thread Handles
pthread_t prod1, prod2, cons;
int id1 = 1, id2 = 2;

// --- UI HELPER FUNCTIONS ---
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printHeader() {
    // High-contrast Cyan for a professional CLI look
    cout << "\033[36m\033[1m"; 
    cout << R"(
  __________________________________________________________
   ____   ____    ____  ___  __  __ _   _ _        _ _____ ___  ____  
  / __ \ / ___|  / ___||_ _||  \/  | | | | |      / \_   _/ _ \|  _ \ 
 | |  | |\___ \  \___ \ | | | |\/| | | | | |     / _ \ | || | | | |_) |
 | |__| | ___) |  ___) || | | |  | | |_| | |___ / ___ \| || |_| |  _ < 
  \____/ |____/  |____/|___||_|  |_|\___/|_____/_/   \_\_| \___/|_| \_\
  __________________________________________________________
    BY JUNAID F2023266884  ||  GITHUB: junicoder
    )" << "\033[0m\n";
}

void display_system_state() {
    cout << "\n\033[35m--- CURRENT SYSTEM STATE ---\033[0m" << endl;
    cout << "Buffer (Waiting): " << "\033[33m" << buffer.size() << "/" << BUFFER_SIZE << "\033[0m" << endl;
    cout << "Ready Queue:      " << "\033[32m" << ready_queue.size() << "\033[0m" << endl;
    cout << "Blocked Queue:    " << "\033[31m" << blocked_queue.size() << "\033[0m" << endl;
    
    if (!available_resources.empty()) {
        cout << "Resources: \033[36m[" 
             << available_resources[0] << ", " 
             << available_resources[1] << ", " 
             << available_resources[2] << "]\033[0m" << endl;
    }
    cout << "\n\033[90mPress Enter to return to menu...\033[0m";
    cin.ignore();
    cin.get();
}

int main() {
    // 1. Initialization
    srand(time(0));
    initialize_system_resources();
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_mutex_init(&mutex_lock, NULL);

    // 2. Start Worker Threads
    pthread_create(&prod1, NULL, producer_thread, &id1);
    pthread_create(&prod2, NULL, producer_thread, &id2);
    pthread_create(&cons, NULL, consumer_thread, NULL);

    // 3. Epic Menu Interface
    string choice;
    while (true) {
        clearScreen();
        printHeader();

        cout << "\n\033[1m  SYSTEM STATUS: \033[32mRUNNING\033[0m\n\n";
        
        cout << "  \033[35m┌── KERNEL COMMANDS ───────────────────┐\033[0m\n";
        cout << "  \033[35m│\033[0m  [1] \033[1mDISPLAY SYSTEM STATE\033[0m         \033[35m│\033[0m\n";
        cout << "  \033[35m│\033[0m  [2] \033[1mRUN SCHEDULER\033[0m                \033[35m│\033[0m\n";
        cout << "  \033[35m│\033[0m  [3] \033[1mFORCE ADD PROCESS (P999)\033[0m     \033[35m│\033[0m\n";
        cout << "  \033[35m└──────────────────────────────────────┘\033[0m\n";
        
        cout << "\n  \033[31m[X] SHUTDOWN\033[0m    \033[33m[R] REFRESH\033[0m\n\n";
        
        cout << "\033[1m\033[36m  junicoder@os-sim\033[0m:\033[33m~\033[0m$ ";
        cin >> choice;

        if (choice == "1") {
            display_system_state();
        }
        else if (choice == "2") {
            cout << "\033[32m  [!] Calling run_scheduler()...\033[0m" << endl;
            pthread_mutex_lock(&mutex_lock);
            run_scheduler();
            pthread_mutex_unlock(&mutex_lock);
            this_thread::sleep_for(chrono::milliseconds(1900));
        }
        else if (choice == "3") {
            Process p;
            p.id = 999; p.burst_time = 5; p.priority = 1;
            p.remaining_time = 5;
            p.arrival_time = time(NULL);
            for(int i=0; i<3; i++) { p.max_need[i]=1; p.need[i]=1; p.allocated[i]=0; }
            
            pthread_mutex_lock(&mutex_lock);
            buffer.push(p); 
            pthread_mutex_unlock(&mutex_lock);
            sem_post(&full_slots);
            cout << "\033[32m  [OK] Process P999 injected into buffer.\033[0m" << endl;
            this_thread::sleep_for(chrono::milliseconds(800));
        }
        else if (choice == "x" || choice == "X") {
            simulation_running = false;
            sem_post(&empty_slots); sem_post(&full_slots);
            pthread_cancel(prod1); pthread_cancel(prod2); pthread_cancel(cons);
            sem_destroy(&empty_slots);
            sem_destroy(&full_slots);
            pthread_mutex_destroy(&mutex_lock);
            cout << "\033[31m  System Halt. Exiting...\033[0m" << endl;
            return 0;
        }
    }
    return 0;
}
