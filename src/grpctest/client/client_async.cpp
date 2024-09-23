#include <proto/address.pb.h>
#include <proto/addressbook.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include <iostream>
#include <coroutine>
#include <thread>
#include <sync_task.hpp>


class AddressClient {
public:
    explicit AddressClient(std::shared_ptr<grpc::Channel> channel):stub_(expcmake::AddressBook::NewStub(channel)){};

    void GetAddress(const std::string& user){
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
    }

    void AsyncCompleteRpc() {
        void* got_tag;
        bool ok = false;

        static int seq = 0;

        // Block until the next result is available in the completion queue "cq".
        while (cq_.Next(&got_tag, &ok)) {
        // The tag in this example is the memory location of the call object
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            // Verify that the request was completed successfully. Note that "ok"
            // corresponds solely to the request for updates introduced by Finish().

            if (call->status.ok())
                std::cout << "Greeter received: " << call->reply.name() << " " << seq++ << std::endl;
            else
                std::cout << "RPC failed" << std::endl;

            // Once we're complete, deallocate the call object.
            delete call;
        }
  }

 private:
    // struct for keeping state and data information
    struct AsyncClientCall {    

    // Container for the data we expect from the server.
        expcmake::Address reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        grpc::ClientContext context;

        // Storage for the status of the RPC upon completion.
        grpc::Status status;

        std::unique_ptr<grpc::ClientAsyncResponseReader<expcmake::Address>> response_reader;
    };
    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<expcmake::AddressBook::Stub> stub_;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    grpc::CompletionQueue cq_;
};


int main(int argc, char* argv[])
{
    // Setup request
    /*expcmake::NameQuerry query;
    expcmake::Address result;
    query.set_name("John");*/

    // Call
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    AddressClient greeter(channel);

    std::thread thread_ = std::thread(&AddressClient::AsyncCompleteRpc, &greeter);
    //std::unique_ptr<expcmake::AddressBook::Stub> stub = expcmake::AddressBook::NewStub(channel);
    //grpc::ClientContext context;
    //grpc::Status status = stub->GetAddress(&context, query, &result);

    for(int i = 0 ; i < 100; i++)
    {
        greeter.GetAddress("John");
    }

    // Output result
    /*std::cout << "I got:" << std::endl;
    std::cout << "Name: " << result.name() << std::endl;
    std::cout << "City: " << result.city() << std::endl;
    std::cout << "Zip:  " << result.zip() << std::endl;
    std::cout << "Street: " << result.street() << std::endl;
    std::cout << "Country: " << result.country() << std::endl;*/
    //greeter.AsyncCompleteRpc();
    std::cout << "Press control-c to quit" << std::endl << std::endl;
    thread_.join(); 

    return 0;
}