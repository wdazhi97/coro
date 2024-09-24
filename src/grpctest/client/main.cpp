#include <proto/address.pb.h>
#include <proto/addressbook.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include <iostream>
#include <coroutine>
#include <thread>
#include <sync_task.hpp>
#include <task.hpp>
#include <vector>
#include <when_all_ready_task.h>


auto start = std::chrono::high_resolution_clock::now();
auto end = std::chrono::high_resolution_clock::now();

struct AsyncClientCall {

    struct awaiter{

        explicit awaiter(AsyncClientCall* call):call_(call){}
        bool await_ready() const {return false;}

        grpc::Status await_resume() {return call_->status;}

        void await_suspend(std::coroutine_handle<> h)
        {
            call_->handle = h;
        }

        AsyncClientCall* call_;
    };
    
    auto operator co_await() {
        return awaiter(this);
    }

    std::coroutine_handle<> handle;

    void resume()
    {
        if(handle)
        {
            handle.resume();
        }
    }

    // Container for the data we expect from the server.
    expcmake::Address reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    grpc::ClientContext context;

    // Storage for the status of the RPC upon completion.
    grpc::Status status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<expcmake::Address>> response_reader;
};


class AddressClient {
public:
    explicit AddressClient(std::shared_ptr<grpc::Channel> channel):stub_(expcmake::AddressBook::NewStub(channel)){};

    AsyncClientCall* GetAddress(const std::string& user){
        expcmake::NameQuerry query;
        expcmake::Address result;
        query.set_name("John");

        AsyncClientCall* call = new AsyncClientCall;
        /*grpc::ClientContext context;
        grpc::CompletionQueue cq;
        grpc::Status status;
        expcmake::Address reply;*/

        call->response_reader = stub_->PrepareAsyncGetAddress(&call->context, query, &cq_);
        call->response_reader->StartCall();
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);
        return call;
    }

    void AsyncCompleteRpc(int total) {
        void* got_tag;
        bool ok = false;

        static int seq = 0;

        // Block until the next result is available in the completion queue "cq".
        while (seq < total && cq_.Next(&got_tag, &ok)) {
        // The tag in this example is the memory location of the call object
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            // Verify that the request was completed successfully. Note that "ok"
            // corresponds solely to the request for updates introduced by Finish().

            /*if (call->status.ok())
                std::cout << "Greeter received: " << call->reply.name() << std::endl;
            else
                std::cout << "RPC failed" << std::endl;*/
            call->resume();
            seq++;

            // Once we're complete, deallocate the call object.
            //delete call;
        }
  }

 private:
    // struct for keeping state and data information
    
    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<expcmake::AddressBook::Stub> stub_;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    grpc::CompletionQueue cq_;
};

cppcoro::task<int> GetAddress(AddressClient & Client)
{
    std::string user = "John";
    auto * call = Client.GetAddress(user);
    grpc::Status status = co_await *call;
    if(call->status.ok()){}
        //std::cout << "Greeter received: " << call->reply.name()  << std::endl;
    else
        std::cout << "RPC failed" << std::endl;
    delete call;
    co_return 0;
}

cppcoro::sync_task<int> GetAddressStart(AddressClient & Client)
{
    co_return co_await GetAddress(Client);
}

cppcoro::sync_task<int> GetAllTest(AddressClient & Client)
{
    std::vector<cppcoro::task<int>> tasks;
    for(int i = 0; i < 10000; ++i)
    {
        tasks.emplace_back(GetAddress(Client));
    }
    co_await cppcoro::when_all_ready(std::move(tasks));
    co_return 0;
}

int main(int argc, char* argv[])
{
    // Setup request
    /*expcmake::NameQuerry query;
    expcmake::Address result;
    query.set_name("John");*/

    // Call
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    AddressClient greeter(channel);

    start = std::chrono::high_resolution_clock::now();
    /*for(int i = 0 ; i < 10000; i++)
    {
    //GetAddressStart(greeter);
        GetAddressStart(greeter);
    }*/
    GetAllTest(greeter);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "执行时间: " << duration.count() << " 秒" << std::endl;
    
    //std::thread thread_ = std::thread(&AddressClient::AsyncCompleteRpc, &greeter);
    //std::unique_ptr<expcmake::AddressBook::Stub> stub = expcmake::AddressBook::NewStub(channel);
    //grpc::ClientContext context;
    //grpc::Status status = stub->GetAddress(&context, query, &result);

    //greeter.GetAddress("John");

    // Output result
    /*std::cout << "I got:" << std::endl;
    std::cout << "Name: " << result.name() << std::endl;
    std::cout << "City: " << result.city() << std::endl;
    std::cout << "Zip:  " << result.zip() << std::endl;
    std::cout << "Street: " << result.street() << std::endl;
    std::cout << "Country: " << result.country() << std::endl;*/
    greeter.AsyncCompleteRpc(10000);
    std::cout << "Press control-c to quit" << std::endl << std::endl;
    //thread_.join(); 

    return 0;
}