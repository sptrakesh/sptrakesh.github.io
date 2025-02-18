# Models
Model structures for making the various types of requests, and receiving the expected
responses are also provided.  Request and response structures accept user define data
models subject to some constraints.  See the model [unit test](https://github.com/sptrakesh/mongo-service/blob/master/test/unit/document.cpp)
suite for details on using the provided models.
* [api](https://github.com/sptrakesh/mongo-service/blob/master/src/api/api.hpp) lower level API for interacting with the service.
* [request](https://github.com/sptrakesh/mongo-service/tree/master/src/api/model/request) root folder with the
  structures that conform to the [protocol](mongo-service.md#protocol).  Models that accept references to user defined data are
  also available when creating/updating data.  See `CreateWithReference` structure in
  [create.hpp](https://github.com/sptrakesh/mongo-service/blob/master/src/api/model/request/create.hpp) for instance.
* [response](https://github.com/sptrakesh/mongo-service/tree/master/src/api/model/response) root folder with structures
  that conform to the service responses.
* [respository](https://github.com/sptrakesh/mongo-service/blob/master/src/api/repository/repository.hpp) repository
  interface for interacting with the service.  See [integration](https://github.com/sptrakesh/mongo-service/blob/master/test/integration/repository.cpp)
  test suite for examples of usage.
