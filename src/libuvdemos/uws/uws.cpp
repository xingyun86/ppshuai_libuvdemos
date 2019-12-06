// uws.cpp : Defines the entry point for the application.
//

#include "uws.h"
#include "App.h"

/* Note that uWS::SSLApp({options}) is the same as uWS::App() when compiled without SSL support */

int main() {
	/* Overly simple hello world app */
	uWS::App()
		.get("/*", [](auto* res, auto* req) {
			res->end("Hello world!");
		})
		.listen(3000, [](auto* token) {
				if (token) {
					std::cout << "Listening on port " << 3000 << std::endl;
				}
		})
		.run();
	std::cout << "Failed to listen on port 3000" << std::endl;
}