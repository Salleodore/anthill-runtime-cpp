
#include "anthill/Utils.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Log.h"

#include "uv.h"
#include "anthill/Websockets.h"

using namespace std::placeholders;

namespace online
{
    WebsocketRPCPtr WebsocketRPC::Create()
    {
        return WebsocketRPCPtr(new WebsocketRPC());
    }
    
    WebsocketRPC::WebsocketRPC() :
        m_connected(false)
    {
        m_client.getDefaultGroup<uWS::CLIENT>().onMessage(std::bind(&WebsocketRPC::onMessage, this, _1, _2, _3, _4));
    }
    
    void WebsocketRPC::close()
    {
        m_client.getDefaultGroup<uWS::CLIENT>().close();
    }
    
    void WebsocketRPC::terminate()
    {
        m_client.getDefaultGroup<uWS::CLIENT>().terminate();
    }
		
	void WebsocketRPC::waitForShutdown()
	{
        uv_loop_t* loop = (uv_loop_t*)m_client.getLoop();
		while (uv_loop_alive(loop))
		{
			update();
		}
	}
    
    WebsocketRPC::~WebsocketRPC()
    {
    }
    
    void WebsocketRPC::connect(const std::string& location, const Options& options,
                               ConnectCallback onConnect, DisconnectCallback onDisconnect,
                               const std::map<std::string, std::string>& extraHeaders)
    {
        std::string ws;
        
        if (location.find("http://") == 0)
        {
            ws = "ws://" + location.substr(7);
        }
        else if (location.find("https://") == 0)
        {
            ws = "wss://" + location.substr(8);
        }
        else
        {
            Log::get() << "Error: bad protocol: " << location << std::endl;
            onConnect(false, 400);
            return;
        }
    
        std::stringstream path;
        path << ws << "?";
        bool second = false;
        
        for (Options::const_iterator it = options.begin(); it != options.end(); it++)
        {
            if (second)
            {
                path << "&";
            }
            else
            {
                second = true;
            }
            
            path << url_encode(it->first) << "=" << url_encode(it->second);
        }
        
        m_client.getDefaultGroup<uWS::CLIENT>().onConnection([=](uWS::WebSocket<uWS::CLIENT> *socket, uWS::HttpRequest request)
        {
            m_socket = socket;
            m_connected = true;
            onConnect(true, 200);
        });
        
        m_client.onDisconnection([=](uWS::WebSocket<uWS::CLIENT> *socket, int code, char * message, size_t length)
        {
            m_connected = false;
            
            rejectAllResponseHandlers(code, "Disconnected", "Rejected, because websocet has been disconnected");
            
            // uWebSockets always returns 'message' as nullptr
            onDisconnect(code, "Disconnected");
            Log::get() << "Websocket disconnected: " << code << std::endl;

			if( socket )
			{
				if( socket->getNodeData() )
				{
					if( socket->getNodeData()->recvBuffer )
					{
						std::string nodeData = socket->getNodeData()->recvBuffer;
						if( nodeData.size() > 4096 )
							nodeData.resize( 4096 );

						const char charset[] =
							"0123456789"
							"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							"abcdefghijklmnopqrstuvwxyz"
							";:[]{}@`,./_?!\"#$%&'()=^~\\|*-+"
							"\n\t";
						auto charsetBegin = std::begin( charset );
						auto charsetEnd = std::end( charset ) - 1;

						for( std::string::iterator	it = nodeData.begin(),
													end	= nodeData.end(); 
													it != end; )
						{
							char c = (*it);
							if( std::find( charsetBegin, charsetEnd, c ) == charsetEnd )
							{
								it = nodeData.erase( it );
								end = nodeData.end();
							}
							else
							{
								++it;
							}
						}

						Log::get() << "Socket Node Data" << std::endl;
						Log::get() << "Data(" << nodeData.size() << "): " << nodeData << std::endl << std::endl;

						if( AnthillRuntime::IsInstanceValid() )
						{
							std::string messageStr = message ? message : "";
							Log::get() << "Processing disconnection info..." << std::endl;

							AnthillRuntime::Instance().processSocketDisconnectionInfo( 
								code,
								"\ncode = " + std::to_string( code ) + 
								"\nmessage = " + messageStr + 
								"\nnodeData = " + nodeData );

							Log::get() << "Disconnection info processed." << std::endl;
						}
					}
				}
			}
        });
        
        m_client.onError([this, onConnect](void *user)
        {
            m_connected = false;
            
            Log::get() << m_client.getDefaultGroup<uWS::CLIENT>().getCloseMessage() << " "
            << m_client.getDefaultGroup<uWS::CLIENT>().getCloseContents() << std::endl;
            onConnect(false, m_client.getDefaultGroup<uWS::CLIENT>().getCloseCode());
        });
        
        m_client.connect(path.str(), nullptr, extraHeaders);
    }
    
    void WebsocketRPC::disconnect(int code, const std::string& reason)
    {
        m_client.getDefaultGroup<uWS::CLIENT>().close(code, (char*)reason.c_str(), reason.size());
        m_client.getDefaultGroup<uWS::CLIENT>().terminate();

		m_connected = false;
    }
    
    void WebsocketRPC::onMessage(uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode)
    {
        std::string data(message, length);
        received(data);
    }
    
    void WebsocketRPC::update()
    {
        uv_loop_t* loop = (uv_loop_t*)m_client.getLoop();
		if (loop)
		{
			uv_run(loop, UV_RUN_NOWAIT);
		}
    }

    bool WebsocketRPC::read(std::string& data)
    {
        return false;
    }
    
    void WebsocketRPC::write(const std::string& data)
    {
        m_socket->send(data.c_str(), data.size(), uWS::OpCode::BINARY);
    }
    
    void WebsocketRPC::error(int code, const std::string& message, const std::string& data)
    {
        Log::get() << "Error occured: " << code << ": " << message << " " << data << std::endl;
    }
}
