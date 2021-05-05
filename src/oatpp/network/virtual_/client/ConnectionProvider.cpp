/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include <future>

#include "ConnectionProvider.hpp"

namespace oatpp { namespace network { namespace virtual_ { namespace client {

ConnectionProvider::ConnectionProvider(const std::shared_ptr<virtual_::Interface>& interface)
  : m_interface(interface)
  , m_maxAvailableToRead(-1)
  , m_maxAvailableToWrite(-1)
{
  setProperty(PROPERTY_HOST, m_interface->getName());
  setProperty(PROPERTY_PORT, "0");
}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<virtual_::Interface>& interface) {
  return std::make_shared<ConnectionProvider>(interface);
}

void ConnectionProvider::stop() {

}

static std::shared_ptr<data::stream::IOStream> getSocket(std::shared_ptr<virtual_::Interface> interface,
                                                         v_io_size maxAvailableToRead,
                                                         v_io_size maxAvailableToWrite) {
  auto submission = interface->connect();
  if(submission->isValid()) {
    auto socket = submission->getSocket();
    if (socket) {
      socket->setOutputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
      socket->setInputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
      socket->setMaxAvailableToReadWrtie(maxAvailableToRead, maxAvailableToWrite);
      return socket;
    }
  }
  throw std::runtime_error("[oatpp::network::virtual_::client::getConnection()]: Error. Can't connect. " + interface->getName()->std_str());
}

std::shared_ptr<data::stream::IOStream> ConnectionProvider::get(const std::chrono::duration<v_int64, std::micro>& timeout) {
  std::packaged_task<std::shared_ptr<data::stream::IOStream>(std::shared_ptr<virtual_::Interface>, v_io_size, v_io_size)> task{getSocket};
  auto future = task.get_future();

  if (timeout == std::chrono::microseconds::zero()) {
    task(m_interface, m_maxAvailableToRead, m_maxAvailableToWrite);
    return future.get();
  }

  std::thread{std::move(task), m_interface, m_maxAvailableToRead, m_maxAvailableToWrite}.detach();
  return future.wait_for(timeout) == std::future_status::ready ? future.get() : nullptr;
}
  
oatpp::async::CoroutineStarterForResult<const std::shared_ptr<oatpp::data::stream::IOStream>&> ConnectionProvider::getAsync(const std::chrono::duration<v_int64, std::micro>& timeout) {
  
  class ConnectCoroutine : public oatpp::async::CoroutineWithResult<ConnectCoroutine, const std::shared_ptr<oatpp::data::stream::IOStream>&> {
  private:
    std::shared_ptr<virtual_::Interface> m_interface;
    v_io_size m_maxAvailableToRead;
    v_io_size m_maxAvailableToWrite;
    std::shared_ptr<virtual_::Interface::ConnectionSubmission> m_submission;
  public:
    
    ConnectCoroutine(const std::shared_ptr<virtual_::Interface>& interface,
                     v_io_size maxAvailableToRead,
                     v_io_size maxAvailableToWrite)
      : m_interface(interface)
      , m_maxAvailableToRead(maxAvailableToRead)
      , m_maxAvailableToWrite(maxAvailableToWrite)
    {}
    
    Action act() override {
      m_submission = m_interface->connectNonBlocking();
      if(m_submission){
        return yieldTo(&ConnectCoroutine::obtainSocket);
      }
      return waitRepeat(std::chrono::milliseconds(100));
    }

    Action obtainSocket() {

      if(m_submission->isValid()) {

        auto socket = m_submission->getSocketNonBlocking();

        if(socket) {
          socket->setOutputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);
          socket->setInputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);
          socket->setMaxAvailableToReadWrtie(m_maxAvailableToRead, m_maxAvailableToWrite);
          return _return(socket);
        }

        return waitRepeat(std::chrono::milliseconds(100));
      }

      return error<Error>("[oatpp::network::virtual_::client::ConnectionProvider::getConnectionAsync()]: Error. Can't connect.");

    }
    
  };
  
  return ConnectCoroutine::startForResult(m_interface, m_maxAvailableToRead, m_maxAvailableToWrite);
  
}
  
}}}}
