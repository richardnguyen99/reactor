# CHANGELOG

## `[0.1.0]` - 2023-07-14

Main focus of this release is to create a simple running program with a minimum
insfrastructure of event loop, task queue, and multithreading. It includes the
implementation and the project structure.

- [x] Initialize the project with CMake, README.md, formatters and other
    development tools. (Commit: [38a1130](https://github.com/richardnguyen99/reactor/commit/38a1130069484a773e295d32df850753e27e5091))
- [x] Create a mininum server that listens and accepts new connections. (Commit:
    [9cd2e91](https://github.com/richardnguyen99/reactor/commit/9cd2e919988b18b8bb80336cd6a986198732c269))
- [x] Add an event loop to monitor file descriptors and event notifications. (Commit:
    [13f2f94](https://github.com/richardnguyen99/reactor/commit/13f2f94c301eec1fb167968ab502e995f511e72c))
- [x] Add a dictionary data structure to store HTTP headers. (Commit:
    [3ad4da7](https://github.com/richardnguyen99/reactor/commit/3ad4da73de4a53873b58661ad04860b26e227ae2))
- [x] Add HTTP request struct and HTTP response struct for reading and sending
    messages. (Commit [eaf1437](https://github.com/richardnguyen99/reactor/commit/eaf1437cbadafc83cf56b9dc8181e633095618a0))
- [x] Add accepting and deleting event handlers in the event loop. (Commit:
    [5281096](https://github.com/richardnguyen99/reactor/commit/5281096daa7edb30ab45f89689ba6e6ddb0727f0))
- [x] Implement a minimum guard check for a valid HTTP request. (Commit:
    [8511010](https://github.com/richardnguyen99/reactor/commit/85110101a12db7a0c554e401531e7e436d5d9fce))
- [x] Add a ring buffer as a task queue for thread pool. (Commit:
    [b3b21b6](https://github.com/richardnguyen99/reactor/commit/b3b21b602a33255cd1b828905c5efa5e91706ef1))
- [x] Add a thread pool to process HTTP requests. (Commit:
    [c499455](https://github.com/richardnguyen99/reactor/commit/c49945599b590c6a11a163600249edbe063da9f6))
- [x] Add static folder and change working directory to the folder. (Commit:
    [40c97c7](https://github.com/richardnguyen99/reactor/commit/40c97c77a8b6f30bd1dfd90571649d670d22fb32))
- [x] Add event and multithreading synchronization. (Commit:
    [ded32c3](https://github.com/richardnguyen99/reactor/commit/ded32c36f4e5aff1d05b65caa3211d757a09b569))

## `[0.2.0]` - 2023-07-23

After setting up the minimum infrastructure to support multiple connections,
this release will focus heavily on the HTTP server itself. This includes
validating HTTP requests, checking resources, checking methods and headers,
handling status and messages, and sending responses.

- [ ] Add check host name header.
- [ ] Add check requested resource.
- [ ] Add check method header.
- [ ] Add check content length header for POST method.
- [ ] Compose HTTP response for event loop to send back to client.
- [ ] Read static files from disk and store their size.
- [ ] Add HTTP response status and messages.
- [ ] Implement logic for `Connection: keep-alive` header.
- [ ] Implement `HEAD` method.
- [ ] Implement `POST` method with forms.
- [ ] Add landing page, global CSS file and entry JavaScript file.
