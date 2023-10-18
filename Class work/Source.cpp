#include<iostream>
#include<conio.h>
#include<thread>

using namespace std;

#define Enter	13
#define Escape	27

#define MIN_TANK_VOLUME	 35
#define MAX_TANK_VOLUME	160
class Tank
{
	const int VOLUME;	//Характеристика объекта
	double fuel_level;	//Состояние объекта
public:
	int get_VOLUME()const
	{
		return VOLUME;
	}
	double get_fuel_level()const
	{
		return fuel_level;
	}
	void fill(double fuel)
	{
		if (fuel < 0)return;
		if (fuel + this->fuel_level > VOLUME)this->fuel_level = VOLUME;
		else this->fuel_level += fuel;
	}
	double give_fuel(double amount)
	{
		fuel_level -= amount;
		if (fuel_level < 0)fuel_level = 0;
		return fuel_level;
	}
	Tank(int volume) :VOLUME(
		volume < MIN_TANK_VOLUME ? MIN_TANK_VOLUME :
		volume > MAX_TANK_VOLUME ? MAX_TANK_VOLUME :
		volume),
		fuel_level(0)
	{
		/*
		if (volume < 35)volume = 35;
		if (volume > 160)volume = 160;
		this->VOLUME = volume;
		*/
		cout << "TConstructor:\t" << this << endl;
	}
	~Tank()
	{
		cout << "TDestructor:\t" << this << endl;
	}

	void info()const
	{
		cout << "Volume:\t" << get_VOLUME() << " liters.\n";
		cout << "Fuel:\t" << get_fuel_level() << " liters.\n";
	}
};

#define MIN_ENGINE_CONSUMPTION	4
#define MAX_ENGINE_CONSUMPTION	30
class Engine
{
	const double CONSUMPTION;
	const double CONSUMPTION_PER_SEC;
	double consumption_per_second;
	bool is_started;
public:
	double get_CONSUMPTION()const
	{
		return CONSUMPTION;
	}
	double get_consumption_per_second()const
	{
		return consumption_per_second;
	}
	void set_consumption_per_second(int speed)
	{
		if (speed == 0)consumption_per_second = CONSUMPTION_PER_SEC; //3*10 в  -5 степени
		else if (0 < speed && speed <= 60)consumption_per_second = CONSUMPTION_PER_SEC * 6.7;
		else if (60 < speed && speed <= 100)consumption_per_second = CONSUMPTION_PER_SEC * 4.7;
		else if (100 < speed && speed <= 140)consumption_per_second = CONSUMPTION_PER_SEC * 6.7;
		else if (140 < speed && speed <= 200)consumption_per_second = CONSUMPTION_PER_SEC * 8.4;
		else if (speed > 200)consumption_per_second = CONSUMPTION_PER_SEC * 10;//3 * 10 в -5й степени
	}
	void start()
	{
		is_started = true;
	}
	void stop()
	{
		is_started = false;
	}
	bool started()const
	{
		return is_started;
	}
	Engine(double consumption) :
		CONSUMPTION
		(
			consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
			consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
			consumption
		), CONSUMPTION_PER_SEC(CONSUMPTION * 3e-5)
	{
		set_consumption_per_second(0);
		is_started = false;
		cout << "EConstructor:\t" << this << endl;
	}
	~Engine()
	{
		cout << "EDestructor:\t" << this << endl;
	}
	void info()const
	{
		cout << "Расход на 100 км.:  " << CONSUMPTION << " л." << endl;
		cout << "Расход за 1 секунду: " << consumption_per_second << " л." << endl;
		cout << "Двигатель " << (is_started ? "запущен" : "остановлен") << endl;
	}
};

#define MAX_SPEED_LOW_LIMIT		120
#define MAX_SPEED_HIGHT_LIMIT	400
#define MIN_ACCELLERATION		 10
#define MAX_ACCELLERATION		 40
class Car
{
	Engine engine;
	Tank tank;
	const int MAX_SPEED;
	const int ACCELERATION;
	int speed;
	bool driver_inside;
	struct
	{
		std::thread panel_thread;			//в этом потоке будет запусткаться метод Panel();
		std::thread engine_idle_thread;
		std::thread free_wheeling_thread;	//силы трения
	} threads;
	//Treads threads;
	bool busy;	//управление занято
public:
	Car(Engine engine, Tank tank, int max_speed = 250, int accelleration = 10) :

		speed(0),
		tank(tank),		//Implicit copy constructor
		engine(engine),	//Implicit copy constructor
		MAX_SPEED
		(
			max_speed < MAX_SPEED_LOW_LIMIT ? MAX_SPEED_LOW_LIMIT :
			max_speed > MAX_SPEED_HIGHT_LIMIT ? MAX_SPEED_HIGHT_LIMIT :
			max_speed
		),
		ACCELERATION
		(
			accelleration < MIN_ACCELLERATION ? MIN_ACCELLERATION :
			accelleration > MAX_ACCELLERATION ? MAX_ACCELLERATION :
			accelleration
		)
	{
		driver_inside = false;
		busy = false;
		cout << "Your car is ready, press Enter to get in" << endl;
	}
	~Car()
	{
		cout << "Car is over :-(" << endl;
	}
	void get_in()
	{
		driver_inside = true;
		threads.panel_thread = std::thread(&Car::panel, this);
		//std::thread(...) - создаем объект класса 'thread' (создаем поток)
		//&Car::panel - указатель на метод (member-function), который будет выполняться в потоке.
		//Для того чтобы в потоке запустит метод, нужно так же указать для какого объекта будет выполняться этот метод
	}
	void get_out()
	{
		system("CLS");
		driver_inside = false;
		//Потоки принято синхронизировать. Метод join() синхронизирует потоки
		if (threads.panel_thread.joinable())threads.panel_thread.join();
		//https://legacy.cplusplus.com/reference/thread/thread/join/
		//https://legacy.cplusplus.com/reference/thread/thread/joinable/
		cout << "You are out" << endl;
	}
	void start()
	{
		engine.start();
		threads.engine_idle_thread = std::thread(&Car::engine_idle, this);
	}
	void stop()
	{
		engine.stop();
		if (threads.engine_idle_thread.joinable())threads.engine_idle_thread.join();
	}
	void accelerate()
	{
		if (driver_inside && engine.started() && !busy)
		{
			speed += ACCELERATION;
			if (speed > MAX_SPEED)speed = MAX_SPEED;
			std::this_thread::sleep_for(1s);
			//busy = false;
			if (speed > 0 && !threads.free_wheeling_thread.joinable())
				threads.free_wheeling_thread = std::thread(&Car::free_wheeling, this);
			busy = true;
		}
	}
	void slow_down()
	{
		if (driver_inside && !busy)
		{
			speed -= ACCELERATION;
			if (speed < 0)speed = 0;
			std::this_thread::sleep_for(1s);
			//busy = false;
			if (speed == 0 && threads.free_wheeling_thread.joinable())
				threads.free_wheeling_thread.join();
			busy = true;
		}
	}
	void control()
	{
		char key;
		do
		{
			key = 0;	//сбрасываем нажатую клавишу
			if (_kbhit())	//Функция _kbhit() возвращает 'true' при нажатии любой клавиши.
				key = _getch();	//Ожидает нажатия клавиши, и возвращает ASCII-код нажатой клавиши
			switch (key)
			{
			case Enter:	driver_inside ? get_out() : get_in();	break;
			case 'F':	//Fill
			case 'f':
				if (driver_inside)
				{
					cout << "Для начала выйдите из машины!";
					break;
				}
				double fuel;
				cout << "Введите желаемый объем топлива: "; cin >> fuel;
				tank.fill(fuel);
				break;
			case 'I':	//Ignition - зажигание
			case 'i':	if (driver_inside)engine.started() ? stop() : start();	break;
			case 'W':
			case 'w':	if (!busy)accelerate();
				/*if (driver_inside)
				{
					speed += ACCELERATION;
					if (speed > MAX_SPEED)speed = MAX_SPEED;
					std::this_thread::sleep_for(1s);
				}*/
				break;
			case 'S':
			case 's':	if (!busy)slow_down();
				/*if (driver_inside)
				{
					speed -= ACCELERATION;
					if (speed < 0)speed = 0;
					std::this_thread::sleep_for(1s);
				}*/
				break;
			case Escape:
				stop();
				get_out();
				cout << "Bye" << endl;
			}
			if (tank.get_fuel_level() == 0)stop();
			/*if (speed > 0 && !threads.free_wheeling_thread.joinable())
				threads.free_wheeling_thread = std::thread(&Car::free_wheeling, this);*/
			if (speed == 0 && threads.free_wheeling_thread.joinable())
				threads.free_wheeling_thread.join();
			//std::this_thread::sleep_for(1s);
		} while (key != Escape);
	}
	void free_wheeling()
	{
		while (--speed > 0)
			std::this_thread::sleep_for(1s);
	}
	void engine_idle()
	{
		while (engine.started() && tank.give_fuel(engine.get_consumption_per_second()))
		{
			std::this_thread::sleep_for(1s);
		if (speed == 0)engine.set_consumption_per_second(0);
		else if (0 < speed && speed <= 60)engine.set_consumption_per_second(60);
		else if (60 < speed && speed  <= 100)engine.set_consumption_per_second(100);
		else if (100 < speed && speed <= 140)engine.set_consumption_per_second(140);
		else if (140 < speed && speed <= 200) engine.set_consumption_per_second(200);
		else if (speed > 200) engine.set_consumption_per_second(250);
			busy = false;
		}
	}
	void panel()
	{
		while (driver_inside)
		{
			system("CLS");
			cout << "Speed:\t" << speed << " km/h\n";
			cout << "Fuel level:\t" << tank.get_fuel_level() << " liters.\n";
			cout << "Engine is" << (engine.started() ? " started" : " stopped") << endl;
			cout << " Consumption Per sec" << engine.get_consumption_per_second() << endl;
			if (tank.get_fuel_level() < 5)cout << "LOW FUEL" << endl;
			std::this_thread::sleep_for(1s);
		}
	}
	void info()const
	{
		engine.info();
		tank.info();
		cout << "Max speed:    \t" << MAX_SPEED << " km/h\n";
		cout << "Acceleration:\t" << ACCELERATION << " km/h\n";
		cout << "Speed:\t\t\t" << speed << " km/h\n";
	}
};

//Deep copy - побитовое копирование
//Shallow copy - поверхностное копирование

//#define TANK_CHECK
//Директива #define (определить) создает макроопределение
//#define ENGINE_CHECK

void main()
{
	setlocale(LC_ALL, "");

#ifdef TANK_CHECK		//показывает начало блока кода
	//Если определено 'TANK_CHECK', то нижеследующий код будет виден компилятору:
	Tank tank(20);
	tank.info();
	while (true)
	{
		double fuel;
		cout << "Введите объем топлива: "; cin >> fuel;
		tank.fill(fuel);
		tank.info();
	}
#endif // TANK_CHECK	//показывает конец блока кода

#ifdef ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif // ENGINE_CHECK

	Car car(10, 60);
	//car.info();
	car.control();
}