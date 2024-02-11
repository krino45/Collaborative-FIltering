// collaborative filtering with pearson correlation.
//
// Film Based means the coefficient is based on the similarity of films's ratings
//
// Critic Based means the coefficient is based on the similarity of critic's reviews.
#include <iostream>
#include <unordered_map>
#include <math.h>
#include <fstream>

#define USERBASE_SIZE 1918 // range [1; 83918]
#define MOVIEBASE_SIZE 40  // range [1; 525]

typedef struct User {

    std::string username;
    std::unordered_map<std::string, double> interest; //std::string - film, double - rating of the film, 0-100

} User;

typedef struct Film {
    int fid;
    std::string filmname;
    std::unordered_map<std::string, double> ratings; //std::string - user, double - rating of the film, 0-100

} Film;

double avg_Rating(Film *film)
{
    double avg_rating = 0;
    int size = 0;

    for (auto &i : film->ratings)
    {
        if (i.second == -1)
            continue;
        avg_rating += i.second;
        size++;
    }
    return avg_rating / size;
}

double Film_Based_Pearson_Correlation_weight(User** userset, Film *film_i, Film *film_j)
{
    double avg_film_i = avg_Rating(film_i);
    double avg_film_j = avg_Rating(film_j);

    double sum1 = 0;
    double sum2 = 0;
    double sum3 = 0;
    double interest = 0;

    for (int i = 0; i < USERBASE_SIZE; i++){
        interest = userset[i]->interest.at(film_i->filmname);
        if (interest == -1)
            continue;
        sum1 += (interest - avg_film_i) * (interest - avg_film_j);
        sum2 += pow(interest - avg_film_i, 2);
        sum3 += pow(interest - avg_film_j, 2);
    }

    double weight = sum1 / (sqrt(sum2) * sqrt(sum3));
    return weight;
}

double Film_Based_Pearson_Correlation_Prediction(User *usr, Film **films, User** userset, Film *film_tbd) // predicts the rating of film_tbd by usr
{
    double avg_film_tbd = avg_Rating(film_tbd);
    Film *film_i;
    double sum1 = 0;
    double sum2 = 0;

    for (auto &itr : usr->interest)
    {
        if (itr.second == -1)
            continue;
        for (int i = 0; i < MOVIEBASE_SIZE; i++){
            if (itr.first == films[i]->filmname) {
                film_i = films[i];
                break; 
            }
        }


        double weight = Film_Based_Pearson_Correlation_weight(userset, film_tbd, film_i);
        sum1 += (itr.second - avg_Rating(film_i)) * weight;
        sum2 += abs(weight);
    }
    double prediction = avg_film_tbd + (sum1 / sum2);
    return prediction;
}

double Critic_Based_Pearson_Correlation(User *user1, User *user2){
    struct user1user2{
        std::string name;
        double rating_u1;
        double rating_u2;
    };

    int u1u2_size = user1->interest.size();
    struct user1user2 **u1u2 = new struct user1user2*[u1u2_size];
    int i = 0;
    int n = 0;

    for (auto &item : user1->interest)
    {
        if(user2->interest.at(item.first) != -1){
            u1u2[i] = new struct user1user2;
            u1u2[i]->name = item.first;
            u1u2[i]->rating_u1 = item.second;
            u1u2[i]->rating_u2 = user2->interest.at(item.first);
            n++;
        }
        else {
            u1u2[i] = new struct user1user2;
            u1u2[i]->name = "";
            u1u2[i]->rating_u1 = 0;
            u1u2[i]->rating_u2 = 0;
        }

        i++;
    }

    if(!n) 
    {
        for (int i = 0; i < u1u2_size; i++)
            delete u1u2[i];
        delete[] u1u2;
        return 0;
    }

    double sum1 = 0;
    double sum2 = 0;
    double sum1Sq = 0;
    double sum2Sq = 0;
    double psum = 0;


    for (int i = 0; i < u1u2_size; i++){
        sum1 += u1u2[i]->rating_u1;
        sum2 += u1u2[i]->rating_u2;
        sum1Sq += pow(u1u2[i]->rating_u1,2);
        sum2Sq += pow(u1u2[i]->rating_u2,2);
        psum += u1u2[i]->rating_u1*u1u2[i]->rating_u2;
    }

    double num = psum - (sum1 * (sum2 / n));
    double denum = sqrt((sum1Sq - (pow(sum1, 2) / n)) * (sum2Sq - (pow(sum2, 2) / n)));
    if (denum == 0 ) return 0;

    for (int i = 0; i < u1u2_size; i++) {
        delete u1u2[i];
    }
    delete[] u1u2;

    return num / denum;
}

void calculateAllCorrelationsForUser(std::string username, User** userbase, double correlation_threshold) {
    std::cout << std::endl;
    std::cout << "cACFU:\tCalculating correlations for user '" << username << "'(correlation threshold = " << correlation_threshold << ")" << std::endl;
    std::cout << std::endl;
    int flag = 1;
    int flag1 = 1;
    User *currentUser;
    for (int i = 0; i < USERBASE_SIZE; i++)
    {
        if(username == userbase[i]->username){
            currentUser = userbase[i];
            flag1 = 0;
            break;
        }
    }
    if(flag1 == 1){
        std::cout << "Err:\tNo user with name '" << username << "' found." << std::endl;
        return;
    }
    std::ofstream file;
    file.open("data.csv");
    file << "correlation;name\n";

    for (int j = 0; j < USERBASE_SIZE; j++)
    {
        if (currentUser != userbase[j]) {
            double correlation = Critic_Based_Pearson_Correlation(currentUser, userbase[j]);
            if (correlation > correlation_threshold) {
                std::cout << "\tCorrelation coefficient between '" << currentUser->username << "' and '" << userbase[j]->username << "' is '" << correlation << std::endl;
                flag = 0;
                file << correlation << ";" << userbase[j]->username << "\n";
            }
        }
    }
    if(flag)
        std::cout << "\t\tNo correlating users with such index were found..." << std::endl;
    else
        std::cout << "\t\tNo more corralating users found!" << std::endl;
    file.close();
}

void calculateCorrelationBetweenUsers(std::string Username1, std::string Username2, User **userbase) {
    std::cout << std::endl;
    std::cout << "cCBUs:\tCalculating the correlation between users " << Username1 << " and " << Username2 <<"." << std::endl;
    std::cout << std::endl;
    if(Username1 == Username2){
        std::cout << "Err:\tCan't calculate correlation between two users with the same name\n\n";
        return;
    }
    int flag = 1;
    int flag1 = 1;
    User *user1;
    User *user2;
    for (int i = 0; i < USERBASE_SIZE; i++)
    {
        if(Username1 == userbase[i]->username){
            user1 = userbase[i];
            flag1 = 0;
            break;
        }
    }

    if(flag1 == 1){
        std::cout << "Err:\tNo user with name " << Username1 << " found." << std::endl;
        return;
    }

    flag1 = 1;

    for (int i = 0; i < USERBASE_SIZE; i++)
    {
        if(Username2 == userbase[i]->username){
            user2 = userbase[i];
            flag1 = 0;
            break;
        }
    }

    if(flag1 == 1){
        std::cout << "Err:\tNo user with name " << Username2 << " found." << std::endl;
        return;
    }

    double correlation = Critic_Based_Pearson_Correlation(user1, user2);
    std::cout << "\tCorrelation coefficient between " << Username1 << " and " << Username2 << " is " << correlation << std::endl;
}


void calculateCorrelationsForUser_FilmBased(std::string movie_name, std::string username, Film **films, User** userset) {
    std::cout << std::endl;
    std::cout << "cCFUFB:\tCalculating predicted Film ratings..." << std::endl;
    std::cout << std::endl;

    Film *currentMovie;
    int flag = 1;
    for (int i = 0; i < MOVIEBASE_SIZE; i++)
    {
        if (films[i]->filmname == movie_name){
            currentMovie = films[i];
            flag = 0;
            break;
        }
    }

    if (flag == 1)
    {
        std::cout << "Err:\tNo movie " << movie_name <<  " found..." << std::endl;
        return;
    }

    int flag1 = 1;
    User *user;
    for (int i = 0; i < USERBASE_SIZE; i++)
    {
        if(username == userset[i]->username){
            user = userset[i];
            flag1 = 0;
            break;
        }
    }

    if(flag1 == 1){
        std::cout << "Err:\tNo user with name " << username << " found." << std::endl;
        return;
    }

    double prediction = Film_Based_Pearson_Correlation_Prediction(user, films, userset, currentMovie);
    std::cout << "\tPredicted rating of " << movie_name << " by " << user->username << " is " << prediction << std::endl;
    if (user->interest.at(movie_name) != -1)
        std::cout << "\t(Actual rating of " << movie_name << " by " << user->username << " is " << user->interest.at(movie_name) << ")" <<  std::endl;
    else
        std::cout << "\t\t(User didnt rate the movie)" << std::endl;
}
        



int main(){

    int option = 0;
    int loadingstate_flag = 0;
    User **userbase = new User *[USERBASE_SIZE];
    Film **filmbase = new Film *[MOVIEBASE_SIZE];
    std::string username1;
    std::string username2;
    std::string username3;
    std::string username4;
    std::string moviename;
    double threshold = -1.1;
    while(option != 9){

        std::cout << "\n\nCollaborative filtering example!\n\n";
        std::cout << "What would you like to do?\n\n";
        std::cout << "1) Find the correlation between two users.\n";
        std::cout << "2) Find all of the correlations above a threshold for a user\n";
        std::cout << "3) Predict the rating that a user is going to give to a film.\n\n";
        std::cout << "9) Exit the program.\n\n";

        std::cout << "Choose option: ";
        std::cin >> option;

        if (option != 9 && loadingstate_flag == 0){

            
            std::ifstream namesfile;

            namesfile.open("names.txt");
            if (!namesfile.is_open()){
                delete[] filmbase;
                delete[] userbase;
                return 0;
            }

            std::cout << "Loading names..." << std::endl;

            for (int i = 0; i < USERBASE_SIZE; i++)
            {
                userbase[i] = new User;
                getline(namesfile, userbase[i]->username);
                // std::cout << userbase[i]->username << std::endl;
            }

            namesfile.close();

           
            std::ifstream filmsfile;
            std::string film_name;

            filmsfile.open("filmnames.txt");
            if(!filmsfile.is_open()) return 0;

            std::cout << "Loading movies..." << std::endl;
            int rating = 0;
            for (int i = 0; i < MOVIEBASE_SIZE; i++)
            {
                filmbase[i] = new Film;
                getline(filmsfile, film_name);
                filmbase[i]->filmname = film_name;
                for (int j = 0; j < USERBASE_SIZE; j++)
                {   
                    if(rand() % 5 == 1) rating = -1;
                    else if (((j+1) % 5) == 0)
                        rating = 6 + rand() % 5; // каждый 5ый более счастливый по жизни
                    else
                        rating = 1 + rand() % 10; //  RANDOMIZER HERE
                    userbase[j]->interest.insert(make_pair(film_name, rating));
                    filmbase[i]->ratings.insert(make_pair(userbase[j]->username, rating));
                }
            
            }

            loadingstate_flag = 1;
        }

        switch(option){
            
            case 1:

                std::cout << "\nInput a username for user 1\n";
                std::cin >> username1;

                std::cout << "\nInput a username for user 2\n";
                std::cin >> username2;

                calculateCorrelationBetweenUsers(username1, username2, userbase);
                break;
            case 2:

                std::cout << "\nInput a username.\n";
                std::cin >> username3;

                threshold = -1.1;
                while (threshold <= -1 || threshold >= 1)
                {
                    std::cout << "\nInput a valid threshold. ( range is [-1 : 1], separator is dot.)\n";
                    std::cin >> threshold;
                }
                calculateAllCorrelationsForUser(username3, userbase, threshold);
                break;

            case 3:

                std::cout << "\nInput a username.\n";
                std::cin >> username4;
                
                std::cout << "\nInput a movie name.\n";
                std::cin.ignore();
                getline(std::cin, moviename);

                calculateCorrelationsForUser_FilmBased(moviename, username4, filmbase, userbase);
                break;

            case 9:
                if(loadingstate_flag == 1)
                {
                    std::cout << std::endl;
                    std::cout << "Deleting the user- and movie- bases..." << std::endl;
                    for (int i = 0; i < USERBASE_SIZE; i++)
                    {
                        delete userbase[i];
                    }
                    
                    std::cout << "\tUserbase deleted." << std::endl;
                    for (int i = 0; i < MOVIEBASE_SIZE; i++)
                    {
                        delete filmbase[i];
                    }
                    
                    std::cout << "\tMoviebase deleted." << std::endl;
                    std::cout << "Finished!" << std::endl;
                    loadingstate_flag = 0;
                }
                break;
            default:
                break;
        }
    }

if(loadingstate_flag == 1)
    {
        std::cout << std::endl;
        std::cout << "Deleting the user- and movie- bases..." << std::endl;
        for (int i = 0; i < USERBASE_SIZE; i++)
        {
            delete userbase[i];
        }
        delete[] userbase;
        std::cout << "\tUserbase deleted." << std::endl;
        for (int i = 0; i < MOVIEBASE_SIZE; i++)
        {
            delete filmbase[i];
        }
        delete[] filmbase;
        std::cout << "\tMoviebase deleted." << std::endl;
        std::cout << "Finished!" << std::endl;
    }
    else{
        delete[] filmbase;
        delete[] userbase;
    }
    return 0;
}
/*
        User **userbase = new User *[USERBASE_SIZE];
        ifstream namesfile;

        namesfile.open("names.txt");
        if(!namesfile.is_open()) return 0;

        std::cout << "Loading names..." << std::endl;

        for (int i = 0; i < USERBASE_SIZE; i++)
        {   
            userbase[i] = new User;
            getline(namesfile, userbase[i]->username);
        // std::cout << userbase[i]->username << std::endl;
        }

        namesfile.close();

        ifstream filmsfile;
        std::string film_name;

        filmsfile.open("filmnames.txt");
        if(!filmsfile.is_open()) return 0;

        std::cout << "Loading movies..." << std::endl;

        for (int i = 0; i < MOVIEBASE_SIZE; i++)
        {
            getline(filmsfile, film_name);
            for (int j = 0; j < USERBASE_SIZE; j++){
                userbase[j]->interest.insert(make_pair(film_name, (rand() % 5 == 1) ? rand() % 11 : 8 + (rand() % 3)));
            }
        }

        std::cout << "Calculating correlations..." << std::endl;

        std::cout << "Correlation coefficient between " << userbase[0]->username << " and " << userbase[1]->username << " is " << Critic_Based_Pearson_Correlation(userbase[0], userbase[1]) << std::endl;

        for (int i = 0; i < USERBASE_SIZE; i++) {
            for (int j = i + 1; j < USERBASE_SIZE; j++) {
                double correlation = Critic_Based_Pearson_Correlation(userbase[i], userbase[j]);
                if (correlation > 0.4) std::cout << "Correlation coefficient between " << userbase[i]->username << " and " << userbase[j]->username << " is " << correlation << std::endl;
            }
        }
        

    calculateCorrelationsForUser(0, userbase, 0.4);

        for (int i = 0; i < USERBASE_SIZE; i++) {
            delete userbase[i];
        }
        delete[] userbase;
        return 0;
    }
*/