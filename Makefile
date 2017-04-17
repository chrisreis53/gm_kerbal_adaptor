# Copyright 2007-2016 United States Government as represented by the
# Administrator of The National Aeronautics and Space Administration.
# No copyright is claimed in the United States under Title 17, U.S. Code.
# All Rights Reserved.



SRCS   = gm_kerbal_adaptor.cpp PublishThread.cpp SubscribeThread.cpp
TARGET = gm_kerbal_adaptor

include ../AppMake.mf

INCS += -I ./websocketpp -I ./curlpp-0.8.1/include/

 LDFLAGS += -lboost_system -lcurl

CXXFLAGS += -std=c++11
